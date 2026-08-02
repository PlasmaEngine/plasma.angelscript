#include <EditorPluginAngelScript/EditorPluginAngelScriptPCH.h>

#include <EditorFramework/Actions/AssetActions.h>
#include <EditorFramework/Actions/ProjectActions.h>
#include <EditorPluginAngelScript/Actions/AngelScriptActions.h>
#include <EditorPluginAngelScript/AngelScriptAsset/AngelScriptAsset.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/Action/CommandHistoryActions.h>
#include <GuiFoundation/Action/DocumentActions.h>
#include <GuiFoundation/Action/EditActions.h>
#include <GuiFoundation/Action/StandardMenus.h>
#include <GuiFoundation/PropertyGrid/PropertyMetaState.h>

void OnLoadPlugin()
{
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
}

void OnUnloadPlugin()
{
  plPropertyMetaState::GetSingleton()->m_Events.RemoveEventHandler(plAngelScriptAssetDocument::PropertyMetaStateEventHandler);

  plAngelScriptActions::UnregisterActions();
}

PL_PLUGIN_ON_LOADED()
{
  OnLoadPlugin();
}

PL_PLUGIN_ON_UNLOADED()
{
  OnUnloadPlugin();
}
