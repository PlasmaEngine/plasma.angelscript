#include <EditorPluginAngelScript/EditorPluginAngelScriptPCH.h>

#include <EditorFramework/Actions/AssetActions.h>
#include <EditorFramework/Actions/ProjectActions.h>
#include <EditorFramework/EngineShell/WindowContent.h>
#include <EditorPluginAngelScript/Actions/AngelScriptActions.h>
#include <EditorPluginAngelScript/AngelScriptAsset/AngelScriptAsset.h>
#include <EditorPluginAngelScript/AngelScriptWindow/AngelScriptWindow.h>
#include <EditorPluginAngelScript/Tests/AngelScriptTestRunner.h>
#include <EditorFramework/Assets/AssetCurator.h>
#include <EditorFramework/EditorApp/EditorApp.h>
#include <EditorFramework/Scripting/ScriptTools.h>
#include <GuiFoundation/UIServices/UIServices.h>
#include <EditorUi/Widgets.h>
#include <Foundation/Configuration/Plugin.h>
#include <GameUiPlugin/Elements/UiElements.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/Action/CommandHistoryActions.h>
#include <GuiFoundation/Action/DocumentActions.h>
#include <GuiFoundation/Action/EditActions.h>
#include <GuiFoundation/Action/StandardMenus.h>
#include <GuiFoundation/PropertyGrid/PropertyMetaState.h>
#include <ToolsFoundation/Project/ToolsProject.h>

static plString s_sIconFolder;

/// Opens an AngelScript problem or stack frame: the asset's Script window at the line, or an included file externally.
static bool OpenAngelScriptLocation(const plScriptDiagnostic& location)
{
  plAssetCurator* pCurator = plAssetCurator::GetSingleton();

  plStringBuilder sDocument;
  plUuid documentGuid;

  for (plStringView sCandidate : {location.m_sAsset.GetView(), location.m_sFile.GetView()})
  {
    if (sCandidate.IsEmpty())
      continue;

    auto pAsset = pCurator->FindSubAsset(sCandidate);
    if (pAsset.isValid() && pAsset->m_pAssetInfo->m_Path.GetAbsolutePath().HasExtension("plAngelScriptAsset"))
    {
      sDocument = pAsset->m_pAssetInfo->m_Path.GetAbsolutePath();
      documentGuid = pAsset->m_pAssetInfo->m_Info->m_DocumentID;
      break;
    }
  }

  if (!sDocument.IsEmpty())
  {
    if (location.m_uiLine > 0)
      plAngelScriptAssetDocumentWindow::RequestReveal(documentGuid, location.m_uiLine);

    plEditorApp::GetSingleton()->OpenDocumentQueued(sDocument);
    return true;
  }

  // An included .as file: VS Code at the line, as the document's own "open" does. `.as` usually has no default
  // program, and Windows would ask the user to pick one.
  if (!location.m_sFile.IsEmpty())
  {
    plStringBuilder sAbsolute;
    if (plFileSystem::ResolvePath(location.m_sFile, &sAbsolute, nullptr).Succeeded())
    {
      plStringBuilder sGoto;
      sGoto.SetFormat("{}:{}", sAbsolute, plMath::Max(location.m_uiLine, 1u));

      const plString args[] = {plToolsProject::GetSingleton()->GetProjectDirectory(), "--goto", sGoto};
      if (plEditorUiServices::OpenInVsCode(args).Failed())
        plEditorUiServices::OpenFileInDefaultProgram(sAbsolute);
      return true;
    }
  }

  return false;
}

// The AngelScript document in the engine shell: the script gets the width, the properties sit on its right.
static void ArrangeAngelScript(plEditorUiDockManager& ref_panels)
{
  ref_panels.DockPanel("AngelScriptAssetDockWidget", "AngelScriptView", plEditorUiDockSide::Right, 0.3f);
}

/// The package's editor icons (":/AssetIcons/..." are files under EditorIcons): the plugin binary sits in
/// <package>/Bin/<config>, both in the source tree and in an installed copy.
static plString GetIconFolder()
{
  plStringBuilder sBinary, sCopy;
  plPlugin::GetPluginPaths("plEditorPluginAngelScript", sBinary, sCopy, 0);

  plStringBuilder sFolder = sBinary.GetFileDirectory();
  sFolder.PathParentDirectory(2);
  sFolder.AppendPath("EditorIcons");
  return sFolder;
}

void OnLoadPlugin()
{
  s_sIconFolder = GetIconFolder();
  plEditorUi::AddIconFolder(s_sIconFolder);

  plAngelScriptActions::RegisterActions();

  // AngelScript
  {
    // Menu Bar
    {
      const char* szMenuBar = "AngelScriptAssetMenuBar";

      plActionMapManager::RegisterActionMap(szMenuBar).IgnoreResult();
      plStandardMenus::MapActions(szMenuBar, plStandardMenuTypes::Default | plStandardMenuTypes::Edit);
      plProjectActions::MapActions(szMenuBar);
      plDocumentActions::MapMenuActions(szMenuBar);
      plAssetActions::MapMenuActions(szMenuBar);
      plCommandHistoryActions::MapActions(szMenuBar);
    }

    // Tool Bar
    {
      const char* szToolBar = "AngelScriptAssetToolBar";
      plActionMapManager::RegisterActionMap(szToolBar).IgnoreResult();
      plAngelScriptActions::MapActions(szToolBar);
    }

    plPropertyMetaState::GetSingleton()->m_Events.AddEventHandler(plAngelScriptAssetDocument::PropertyMetaStateEventHandler);
  }

  plWindowContentProvider::SetDefaultArrangement("AngelScript", &ArrangeAngelScript);

  if (plScriptTools* pTools = plScriptTools::GetSingleton())
    pTools->RegisterLocationOpener("AngelScript", plScriptTools::LocationOpener(&OpenAngelScriptLocation));

  plRegisterAngelScriptTestRunner();
}

void OnUnloadPlugin()
{
  plUnregisterAngelScriptTestRunner();

  if (plScriptTools* pTools = plScriptTools::GetSingleton())
    pTools->UnregisterLocationOpener("AngelScript");

  plWindowContentProvider::SetDefaultArrangement("AngelScript", nullptr);

  plPropertyMetaState::GetSingleton()->m_Events.RemoveEventHandler(plAngelScriptAssetDocument::PropertyMetaStateEventHandler);
  plUiCodeLanguage::Unregister("AngelScript");

  plAngelScriptActions::UnregisterActions();

  plEditorUi::RemoveIconFolder(s_sIconFolder);
  s_sIconFolder.Clear();
}

PL_PLUGIN_ON_LOADED()
{
  OnLoadPlugin();
}

PL_PLUGIN_ON_UNLOADED()
{
  OnUnloadPlugin();
}
