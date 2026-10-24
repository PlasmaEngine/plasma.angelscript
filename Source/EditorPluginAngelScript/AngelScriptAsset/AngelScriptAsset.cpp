#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <EditorFramework/GUI/ExposedParameters.h>
#include <EditorFramework/Scripting/ScriptFieldMapper.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <EditorPluginAngelScript/AngelScriptAsset/AngelScriptAsset.h>
#include <Foundation/CodeUtils/Preprocessor.h>
#include <GuiFoundation/PropertyGrid/PropertyMetaState.h>
#include <GuiFoundation/UIServices/UIServices.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>
#include <ToolsFoundation/VisualGraph/VisualGraphCommandAccessor.h>

namespace
{
  /// Returns the `///` comment lines directly above the declaration of a class member, without the markers.
  plString FindFieldComment(plStringView sCode, plStringView sName)
  {
    plHybridArray<plStringView, 256> lines;
    sCode.Split(true, lines, "\n");

    for (plUInt32 i = 0; i < lines.GetCount(); ++i)
    {
      plStringView line = lines[i];
      line.Trim(" \t\r");
      if (line.StartsWith("//"))
        continue;

      const char* pName = line.FindWholeWord(plStringBuilder(sName).GetData(), plStringUtils::IsIdentifierDelimiter_C_Code);
      if (pName == nullptr || pName == line.GetStartPointer())
        continue;

      plStringView after(pName + sName.GetElementCount(), line.GetEndPointer());
      after.Trim(" \t");
      if (!after.StartsWith("=") && !after.StartsWith(";"))
        continue;

      plUInt32 uiFirst = i;
      while (uiFirst > 0)
      {
        plStringView above = lines[uiFirst - 1];
        above.Trim(" \t\r");
        if (!above.StartsWith("///"))
          break;
        --uiFirst;
      }

      plStringBuilder comment;
      for (plUInt32 c = uiFirst; c < i; ++c)
      {
        plStringView text = lines[c];
        text.Trim(" \t\r");
        text.Shrink(3, 0);
        comment.Append(text, "\n");
      }
      return comment;
    }

    return {};
  }
} // namespace

// clang-format off
PL_BEGIN_STATIC_REFLECTED_ENUM(plAngelScriptCodeMode, 1)
  PL_ENUM_CONSTANTS(plAngelScriptCodeMode::Inline, plAngelScriptCodeMode::FromFile)
PL_END_STATIC_REFLECTED_ENUM;

PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plAngelScriptParameter, 1, plRTTIDefaultAllocator<plAngelScriptParameter>)
{
  PL_BEGIN_PROPERTIES
  {
    PL_MEMBER_PROPERTY("Name", m_sName)->AddAttributes(new plReadOnlyAttribute()),
    PL_MEMBER_PROPERTY("Declaration", m_sDeclaration)->AddAttributes(new plReadOnlyAttribute()),
    PL_MEMBER_PROPERTY("DefaultValue", m_DefaultValue)->AddAttributes(new plHiddenAttribute()),
    PL_MEMBER_PROPERTY("Expose", m_bExpose),
  }
  PL_END_PROPERTIES;
}
PL_END_DYNAMIC_REFLECTED_TYPE;

PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plAngelScriptAssetProperties, 1, plRTTIDefaultAllocator<plAngelScriptAssetProperties>)
{
  PL_BEGIN_PROPERTIES
  {
    PL_ENUM_MEMBER_PROPERTY("Source", plAngelScriptCodeMode, m_CodeMode),
    PL_MEMBER_PROPERTY("SourceFile", m_sScriptFile)->AddAttributes(new plFileBrowserAttribute("Select Script", "*.as", {}, "AngelScript")),
    PL_MEMBER_PROPERTY("ClassName", m_sClassName)->AddAttributes(new plDefaultValueAttribute("ScriptObject")),
    PL_ARRAY_MEMBER_PROPERTY("Parameters", m_Parameters)->AddAttributes(new plContainerAttribute(false, false, false)),
    PL_ARRAY_MEMBER_PROPERTY("Dependencies", m_Dependencies)->AddAttributes(new plContainerAttribute(false, false, false), new plReadOnlyAttribute()),
    PL_MEMBER_PROPERTY("Code", m_sCode)->AddAttributes(new plHiddenAttribute(), new plDefaultValueAttribute("class ScriptObject : plAngelScriptClass\n\
{\n\t// int PublicIntVar = 0;\n\n\tvoid OnSimulationStarted()\n\t{\n\t\t// plLog::Info(\"Simulation Started\");\n\t}\n\n\t// void Update() { }\n\n\t// void OnMsgTriggerTriggered(plMsgTriggerTriggered@ msg) { }\n}")),
  }
  PL_END_PROPERTIES;
}
PL_END_DYNAMIC_REFLECTED_TYPE;

PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plAngelScriptAssetDocument, 3, plRTTINoAllocator)
PL_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

plAngelScriptAssetDocument::plAngelScriptAssetDocument(plStringView sDocumentPath)
  : plSimpleAssetDocument<plAngelScriptAssetProperties>(sDocumentPath, plAssetDocEngineConnection::Simple)
{
}

plTransformStatus plAngelScriptAssetDocument::InternalTransformAsset(plStreamWriter& stream, plStringView sOutputTag, const plPlatformProfile* pAssetProfile, const plAssetFileHeader& AssetHeader, plBitflags<plTransformFlags> transformFlags)
{
  PL_ASSERT_NOT_IMPLEMENTED;
  return plTransformStatus("");
}

class plAsPreprocessor2
{
public:
  plStringBuilder m_sRefFilePath;
  plStringBuilder m_sMainCode;
  plSet<plString> m_Dependencies;

  plAsPreprocessor2()
  {
    m_Processor.SetFileOpenFunction(plMakeDelegate(&plAsPreprocessor2::PreProc_OpenFile, this));
    m_Processor.SetImplicitPragmaOnce(true);
  }

  plResult Process()
  {
    plStringBuilder sResult;
    return m_Processor.Process(m_sRefFilePath, sResult, false);
  };

private:
  plResult PreProc_OpenFile(plStringView sAbsFile, plDynamicArray<plUInt8>& out_Content, plTimestamp& out_FileModification)
  {
    if (sAbsFile == m_sRefFilePath)
    {
      out_Content.SetCount(m_sMainCode.GetElementCount());
      plMemoryUtils::RawByteCopy(out_Content.GetData(), m_sMainCode.GetData(), m_sMainCode.GetElementCount());
      return PL_SUCCESS;
    }

    plFileReader file;
    if (file.Open(sAbsFile).Failed())
      return PL_FAILURE;

    m_Dependencies.Insert(sAbsFile);

    out_Content.SetCountUninitialized((plUInt32)file.GetFileSize());
    file.ReadBytes(out_Content.GetData(), out_Content.GetCount());
    return PL_SUCCESS;
  }

  plPreprocessor m_Processor;
};

plTransformStatus plAngelScriptAssetDocument::InternalTransformAsset(const char* szTargetFile, plStringView sOutputTag, const plPlatformProfile* pAssetProfile, const plAssetFileHeader& AssetHeader, plBitflags<plTransformFlags> transformFlags)
{
  SyncInfos();

  const bool bCanModify = !transformFlags.IsSet(plTransformFlags::BackgroundProcessing);

  auto pProps = GetProperties();

  plAsPreprocessor2 preProc;

  if (pProps->m_CodeMode == plAngelScriptCodeMode::Inline)
  {
    preProc.m_sRefFilePath = GetDocumentPath();
    preProc.m_sMainCode = pProps->m_sCode;

    plEditorApp::GetSingleton()->MakePathDataDirectoryRelative(preProc.m_sRefFilePath);
  }
  else
  {
    preProc.m_sRefFilePath = pProps->m_sScriptFile;

    plFileReader file;
    if (file.Open(pProps->m_sScriptFile).Succeeded())
    {
      preProc.m_sMainCode.ReadAll(file);
    }
  }

  if (preProc.Process().Succeeded())
  {
    plHybridArray<plString, 16> newDeps;
    for (const auto& str : preProc.m_Dependencies)
    {
      newDeps.PushBack(str);
    }

    if (pProps->m_Dependencies != newDeps)
    {
      if (!bCanModify)
        return plTransformResult::NeedsImport;

      auto pPropObj = GetPropertyObject();

      plCommandHistory* history = GetCommandHistory();
      plObjectCommandAccessor accessor(history);

      accessor.StartTransaction("Update Dependencies");

      // clear the entire array
      accessor.ClearByName(pPropObj, "Dependencies").AssertSuccess();

      const plAbstractProperty* pPropDeps = pPropObj->GetType()->FindPropertyByName("Dependencies");

      // and fill it again
      for (plUInt32 clip = 0; clip < newDeps.GetCount(); ++clip)
      {
        accessor.InsertValue(pPropObj, pPropDeps, newDeps[clip], -1).AssertSuccess();
      }

      accessor.FinishTransaction();
    }
  }

  return plAssetDocument::RemoteExport(AssetHeader, szTargetFile);
}

void plAngelScriptAssetDocument::SyncInfos()
{
  auto pProps = GetProperties();

  if (pProps->m_CodeMode == plAngelScriptCodeMode::FromFile)
  {
    plDocumentConfigMsgToEngine cfg;
    cfg.m_sWhatToDo = "InputFile";
    cfg.m_sValue = pProps->m_sScriptFile;
    GetEditorEngineConnection()->SendMessage(&cfg);
  }
  else
  {
    {
      plStringBuilder sStartFile = GetDocumentPath();
      plEditorApp::GetSingleton()->MakePathDataDirectoryRelative(sStartFile);
      sStartFile.Prepend(":inline:");

      plDocumentConfigMsgToEngine cfg;
      cfg.m_sWhatToDo = "InputFile";
      cfg.m_sValue = sStartFile;
      GetEditorEngineConnection()->SendMessage(&cfg);
    }

    {
      plDocumentConfigMsgToEngine cfg;
      cfg.m_sWhatToDo = "Code";
      cfg.m_sValue = pProps->m_sCode;
      GetEditorEngineConnection()->SendMessage(&cfg);
    }
  }

  {
    plDocumentConfigMsgToEngine cfg;
    cfg.m_sWhatToDo = "Class";
    cfg.m_sValue = pProps->m_sClassName;
    GetEditorEngineConnection()->SendMessage(&cfg);
  }
}

void plAngelScriptAssetDocument::UpdateAssetDocumentInfo(plAssetDocumentInfo* pInfo) const
{
  SUPER::UpdateAssetDocumentInfo(pInfo);

  plExposedParameters* pExposedParams = PL_DEFAULT_NEW(plExposedParameters);

  // Field comments carry the editor metadata (tooltip, @range, @group, @asset, ...).
  plStringBuilder sCode;
  if (GetProperties()->m_CodeMode == plAngelScriptCodeMode::Inline)
  {
    sCode = GetProperties()->m_sCode;
  }
  else
  {
    plFileReader file;
    if (file.Open(GetProperties()->m_sScriptFile).Succeeded())
      sCode.ReadAll(file);
  }

  for (const auto& p : GetProperties()->m_Parameters)
  {
    if (p.m_bExpose == false || p.m_sName.IsEmpty() || !p.m_DefaultValue.IsValid())
      continue;

    plScriptFieldDesc field;
    field.m_sName = p.m_sName;
    field.m_DefaultValue = p.m_DefaultValue;

    // A handle field is picked as an object in the scene; the component resolves it back to a handle.
    if (p.m_DefaultValue.IsA<plGameObjectHandle>())
    {
      field.m_DefaultValue = plString();
      field.m_bObjectReference = true;
    }

    plScriptFieldMapper::ParseCommentTags(FindFieldComment(sCode, p.m_sName), field);

    pExposedParams->m_Parameters.PushBack(plScriptFieldMapper::CreateParameter(field));
  }

  for (const auto& p : GetProperties()->m_Dependencies)
  {
    pInfo->m_TransformDependencies.Insert(p);
  }

  // Info takes ownership of meta data.
  pInfo->m_MetaInfo.PushBack(pExposedParams);
}

void plAngelScriptAssetDocument::OpenExternalEditor()
{
  plStringBuilder sScriptFile(GetProperties()->m_sScriptFile);

  if (GetProperties()->m_sScriptFile.IsEmpty())
  {
    ShowDocumentStatus("No script file specified.");
    return;
  }

  if (!plFileSystem::ExistsFile(sScriptFile))
  {
    ShowDocumentStatus("Script file doesn't exist.");
    return;
  }

  plStringBuilder sScriptFileAbs;
  if (plFileSystem::ResolvePath(sScriptFile, &sScriptFileAbs, nullptr).Failed())
    return;

  {
    const plString args[] = {plToolsProject::GetSingleton()->GetProjectDirectory(), sScriptFileAbs};

    if (plEditorUiServices::OpenInVsCode(args).Failed())
    {
      // try again with a different program
      plEditorUiServices::OpenFileInDefaultProgram(sScriptFileAbs.GetData());
    }
  }
}

void plAngelScriptAssetDocument::SyncExposedParameters()
{
  SyncInfos();

  plDocumentConfigMsgToEngine cfg;
  cfg.m_sWhatToDo = "SyncExposedParams";
  GetEditorEngineConnection()->SendMessage(&cfg);
}

void plAngelScriptAssetDocument::PropertyMetaStateEventHandler(plPropertyMetaStateEvent& e)
{
  if (e.m_pObject->GetTypeAccessor().GetType() == plGetStaticRTTI<plAngelScriptAssetProperties>())
  {
    const plInt64 sourceMode = e.m_pObject->GetTypeAccessor().GetValue("Source").ConvertTo<plInt64>();

    auto& props = *e.m_pPropertyStates;

    props["SourceFile"].m_Visibility = (sourceMode == plAngelScriptCodeMode::Inline) ? plPropertyUiState::Invisible : plPropertyUiState::Default;
  }
}
