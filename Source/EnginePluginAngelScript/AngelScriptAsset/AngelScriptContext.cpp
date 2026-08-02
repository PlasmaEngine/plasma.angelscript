#include <EnginePluginAngelScript/EnginePluginAngelScriptPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Runtime/AsEngineSingleton.h>
#include <AngelScriptPlugin/Runtime/AsInstance.h>
#include <AngelScriptPlugin/Utils/AngelScriptUtils.h>
#include <EnginePluginAngelScript/AngelScriptAsset/AngelScriptContext.h>
#include <Foundation/IO/CompressedStreamZstd.h>
#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/Utilities/AssetFileHeader.h>

// clang-format off
PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plAngelScriptDocumentContext, 1, plRTTIDefaultAllocator<plAngelScriptDocumentContext>)
{
  PL_BEGIN_PROPERTIES
  {
    PL_CONSTANT_PROPERTY("DocumentType", (const char*) "AngelScript"),
  }
  PL_END_PROPERTIES;
}
PL_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

static plAtomicInteger32 s_iCompileCounter;

plAngelScriptDocumentContext::plAngelScriptDocumentContext()
  : plEngineProcessDocumentContext(plEngineProcessDocumentContextFlags::CreateWorld)
{
}

plAngelScriptDocumentContext::~plAngelScriptDocumentContext() = default;

plStatus plAngelScriptDocumentContext::ExportDocument(const plExportDocumentMsgToEngine* pMsg)
{
  plLogSystemToBuffer logBuffer;
  logBuffer.SetLogLevel(plLogMsgType::ErrorMsg);

  plLogSystemScope logScope(&logBuffer);

  plStringBuilder sCode;
  asIScriptModule* pModule = CompileModule(sCode, nullptr);

  if (pModule == nullptr)
  {
    return plStatus(logBuffer.m_sBuffer.GetView());
  }

  plHybridArray<plUInt8, 1024 * 8> bytecode;
  plAngelScriptUtils::SaveByteCode(pModule, bytecode);
  pModule->Discard();

  plDeferredFileWriter out;
  out.SetOutput(pMsg->m_sOutputFile);

  {
    // File Header
    plAssetFileHeader header;
    header.SetFileHashAndVersion(pMsg->m_uiAssetHash, pMsg->m_uiVersion);
    header.Write(out).AssertSuccess();

    plUInt8 uiVersion = 4;
    out << uiVersion;
  }

  plUInt8 uiCompressionMode = 0;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
  uiCompressionMode = 1;
  plCompressedStreamWriterZstd stream(&out, 0, plCompressedStreamWriterZstd::Compression::Average);
#else
  plStreamWriter& stream = out;
#endif

  // write uncompressed
  out << uiCompressionMode;

  // write the rest compressed
  stream << m_sClass;
  stream.WriteArray(bytecode).AssertSuccess();
  stream << sCode;

  return plStatus(PL_SUCCESS);
}

void plAngelScriptDocumentContext::HandleMessage(const plEditorEngineDocumentMsg* pMsg)
{
  if (const plDocumentConfigMsgToEngine* pMsg2 = plDynamicCast<const plDocumentConfigMsgToEngine*>(pMsg))
  {
    if (pMsg2->m_sWhatToDo == "InputFile")
    {
      m_sInputFile = pMsg2->m_sValue;
    }
    else if (pMsg2->m_sWhatToDo == "Code")
    {
      m_sCode = pMsg2->m_sValue;
    }
    else if (pMsg2->m_sWhatToDo == "Class")
    {
      m_sClass = pMsg2->m_sValue;
    }
    else if (pMsg2->m_sWhatToDo == "SyncExposedParams")
    {
      SyncExposedParameters();
    }
    else if (pMsg2->m_sWhatToDo == "RetrieveScriptInfos")
    {
      RetrieveScriptInfos(pMsg2->m_sValue);
    }
  }

  plEngineProcessDocumentContext::HandleMessage(pMsg);
}

plEngineProcessViewContext* plAngelScriptDocumentContext::CreateViewContext()
{
  PL_ASSERT_NOT_IMPLEMENTED;
  return nullptr;
}

void plAngelScriptDocumentContext::DestroyViewContext(plEngineProcessViewContext* pContext)
{
  PL_ASSERT_NOT_IMPLEMENTED;
}

class plLogSystemNull : public plLogInterface
{
public:
  void HandleLogMessage(const plLoggingEventData& le) override
  {
  }
};

void plAngelScriptDocumentContext::SyncExposedParameters()
{
  plLogSystemNull logNull;
  plLogSystemScope scope(&logNull); // disable logging

  plStringBuilder sCode;
  plSet<plString> dependencies;
  asIScriptModule* pModule = CompileModule(sCode, &dependencies);
  if (pModule == nullptr)
    return;

  PL_SCOPE_EXIT(pModule->Discard());

  const asITypeInfo* pClassType = pModule->GetTypeInfoByName(m_sClass);

  asIScriptContext* pContext = pModule->GetEngine()->CreateContext();
  PL_SCOPE_EXIT(pContext->Release());

  AS_CHECK(pContext->Prepare(pClassType->GetFactoryByIndex(0)));
  AS_CHECK(pContext->Execute());
  asIScriptObject* pInstance = (asIScriptObject*)pContext->GetReturnObject();
  pInstance->AddRef();
  PL_SCOPE_EXIT(pInstance->Release());

  plStringBuilder sTypeName;

  {
    plSimpleDocumentConfigMsgToEditor msg;
    msg.m_DocumentGuid = m_DocumentGuid;
    msg.m_sWhatToDo = "SyncExposedParams_Clear";
    SendProcessMessage(&msg);
  }

  for (plUInt32 idx = 0; idx < pClassType->GetPropertyCount(); ++idx)
  {
    const char* szName;
    int typeId;

    bool isPrivate = false, isProtected = false, isReference = false;
    pClassType->GetProperty(idx, &szName, &typeId, &isPrivate, &isProtected, nullptr, &isReference);

    if (isPrivate || isProtected)
      continue;

    plVariant defVal;
    if (plAngelScriptUtils::ReadFromAsTypeAtLocation(pModule->GetEngine(), typeId, pInstance->GetAddressOfProperty(idx), defVal).Succeeded())
    {
      plSimpleDocumentConfigMsgToEditor msg;
      msg.m_DocumentGuid = m_DocumentGuid;
      msg.m_sWhatToDo = "SyncExposedParams_Add";
      msg.m_sPayload = szName;
      msg.m_PayloadValue = defVal;
      SendProcessMessage(&msg);
    }
  }

  for (const auto& dep : dependencies)
  {
    plSimpleDocumentConfigMsgToEditor msg;
    msg.m_DocumentGuid = m_DocumentGuid;
    msg.m_sWhatToDo = "SyncDependencies_Add";
    msg.m_sPayload = dep;
    SendProcessMessage(&msg);
  }

  {
    plSimpleDocumentConfigMsgToEditor msg;
    msg.m_DocumentGuid = m_DocumentGuid;
    msg.m_sWhatToDo = "SyncExposedParams_Finish";
    SendProcessMessage(&msg);
  }
}

asIScriptModule* plAngelScriptDocumentContext::CompileModule(plStringBuilder& out_sCode, plSet<plString>* out_pDependencies)
{
  plStringBuilder sCode, sInputFile;

  if (m_sInputFile.StartsWith(":inline:"))
  {
    sCode = m_sCode;
    sInputFile = m_sInputFile;
    sInputFile.TrimWordStart(":inline:");
  }
  else
  {
    if (m_sInputFile.IsEmpty())
    {
      plLog::Error("No AngelScript file specified.");
      return nullptr;
    }

    plFileReader file;
    if (file.Open(m_sInputFile).Failed())
    {
      plLog::Error("Failed to open script file '{}'.", m_sInputFile);
      return nullptr;
    }

    sCode.ReadAll(file);

    sInputFile = m_sInputFile;
  }

  if (sCode.IsEmpty())
  {
    plLog::Error("Script code is empty.");
    return nullptr;
  }

  plStringBuilder sTempName;
  sTempName.SetFormat("asTempModule-{}", s_iCompileCounter.Increment());

  auto pAs = plAngelScriptEngineSingleton::GetSingleton();
  auto pModule = pAs->CompileModule(sTempName, m_sClass, sInputFile, sCode, &out_sCode, out_pDependencies);

  if (pModule == nullptr)
    return nullptr;

  if (pAs->ValidateModule(pModule).Failed())
  {
    pModule->Discard();
    return nullptr;
  }

  return pModule;
}

static void WriteSet(plStringView sFile, const plSet<plString>& set)
{
  plFileWriter writer;
  if (writer.Open(sFile).Failed())
    return;

  const char* szLineBreak = "\n";

  for (const plString& sItem : set)
  {
    if (sItem.IsEmpty())
      continue;

    writer.WriteBytes(sItem.GetData(), sItem.GetElementCount()).AssertSuccess();
    writer.WriteBytes(szLineBreak, 1).AssertSuccess();
  }
}

void plAngelScriptDocumentContext::RetrieveScriptInfos(plStringView sBasePath)
{
  auto pEngine = plAngelScriptEngineSingleton::GetSingleton()->GetEngine();

  {
    plAsInfos infos;
    plAngelScriptUtils::RetrieveAsInfos(pEngine, infos);

    plStringBuilder sFullPath;

    sFullPath.SetPath(sBasePath, "Types.txt");
    WriteSet(sFullPath, infos.m_Types);

    sFullPath.SetPath(sBasePath, "Namespaces.txt");
    WriteSet(sFullPath, infos.m_Namespaces);

    sFullPath.SetPath(sBasePath, "GlobalFunctions.txt");
    WriteSet(sFullPath, infos.m_GlobalFunctions);

    sFullPath.SetPath(sBasePath, "Methods.txt");
    WriteSet(sFullPath, infos.m_Methods);

    sFullPath.SetPath(sBasePath, "Properties.txt");
    WriteSet(sFullPath, infos.m_Properties);

    sFullPath.SetPath(sBasePath, "Enums.txt");
    WriteSet(sFullPath, infos.m_EnumValues);

    sFullPath.SetPath(sBasePath, "AllDeclarations.txt");
    WriteSet(sFullPath, infos.m_AllDeclarations);

    sFullPath.SetPath(sBasePath, "NotRegisteredDecls.txt");
    WriteSet(sFullPath, plAngelScriptEngineSingleton::GetSingleton()->GetNotRegistered());
  }

  {
    plStringBuilder sPredef;
    plAngelScriptUtils::GenerateAsPredefinedFile(pEngine, sPredef);

    plStringBuilder sFullPath;
    sFullPath.SetPath(sBasePath, "../../as.predefined");
    plFileWriter file;
    if (file.Open(sFullPath).Succeeded())
    {
      file.WriteBytes(sPredef.GetData(), sPredef.GetElementCount()).AssertSuccess();
    }
  }
}
