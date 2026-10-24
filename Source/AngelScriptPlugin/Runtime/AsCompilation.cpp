#include <AngelScriptPlugin/AngelScriptPluginPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Runtime/AsEngineSingleton.h>
#include <Foundation/CodeUtils/Preprocessor.h>
#include <Foundation/IO/FileSystem/FileReader.h>

class plAsPreprocessor
{
public:
  plStringView m_sRefFilePath;
  plStringView m_sMainCode;
  plSet<plString>* m_pDependencies = nullptr;

  plAsPreprocessor()
  {
    m_Processor.SetFileOpenFunction(plMakeDelegate(&plAsPreprocessor::PreProc_OpenFile, this));
    m_Processor.m_ProcessingEvents.AddEventHandler(plMakeDelegate(&plAsPreprocessor::PreProc_Event, this));
    m_Processor.SetImplicitPragmaOnce(true);
    m_Processor.SetPassThroughLine(true);
  }

  plResult Process(plStringBuilder& ref_sResult)
  {
    const bool bNeedsLineStmts = !m_sMainCode.StartsWith("//#ln");
    const bool bKeepComments = !bNeedsLineStmts;

    auto res = m_Processor.Process(m_sRefFilePath, ref_sResult, bKeepComments, false, bNeedsLineStmts);
    ref_sResult.ReplaceAll("#line", "//#ln");
    return res;
  };

private:
  plResult PreProc_OpenFile(plStringView sAbsFile, plDynamicArray<plUInt8>& out_Content, plTimestamp& out_FileModification)
  {
    if (sAbsFile == m_sRefFilePath)
    {
      out_Content.SetCount(m_sMainCode.GetElementCount());
      plMemoryUtils::RawByteCopy(out_Content.GetData(), m_sMainCode.GetStartPointer(), m_sMainCode.GetElementCount());
      return PL_SUCCESS;
    }

    plFileReader file;
    if (file.Open(sAbsFile).Failed())
      return PL_FAILURE;

    if (m_pDependencies)
    {
      m_pDependencies->Insert(sAbsFile);
    }

    out_Content.SetCountUninitialized((plUInt32)file.GetFileSize());
    file.ReadBytes(out_Content.GetData(), out_Content.GetCount());
    return PL_SUCCESS;
  }

  void PreProc_Event(const plPreprocessor::ProcessingEvent& event)
  {
    if (plDynamicArray<plScriptDiagnostic>* pCollector = plAngelScriptEngineSingleton::GetDiagnosticCollector())
    {
      if (event.m_Type == plPreprocessor::ProcessingEvent::Error || event.m_Type == plPreprocessor::ProcessingEvent::Warning)
      {
        plScriptDiagnostic& diag = pCollector->ExpandAndGetRef();
        diag.m_sLanguage = "AngelScript";
        diag.m_Severity = event.m_Type == plPreprocessor::ProcessingEvent::Error ? plScriptDiagnosticSeverity::Error : plScriptDiagnosticSeverity::Warning;
        diag.m_Origin = plScriptDiagnosticOrigin::Compile;
        diag.m_sCode = "Preprocessor";
        diag.m_sMessage = event.m_sInfo;
        if (event.m_pToken != nullptr)
        {
          diag.m_sFile = event.m_pToken->m_File.GetString();
          diag.m_uiLine = event.m_pToken->m_uiLine;
          diag.m_uiColumn = event.m_pToken->m_uiColumn;
        }
      }
    }

    switch (event.m_Type)
    {
      case plPreprocessor::ProcessingEvent::Error:
        plLog::Error("{0}: Line {1} [{2}]: {}", event.m_pToken->m_File.GetString(), event.m_pToken->m_uiLine, event.m_pToken->m_uiColumn, event.m_sInfo);
        break;
      case plPreprocessor::ProcessingEvent::Warning:
        plLog::Warning("{0}: Line {1} [{2}]: {}", event.m_pToken->m_File.GetString(), event.m_pToken->m_uiLine, event.m_pToken->m_uiColumn, event.m_sInfo);
        break;
      default:
        break;
    }
  }

  plPreprocessor m_Processor;
};

void plAngelScriptEngineSingleton::FindCorrectSectionAndLine(const plDynamicArray<plStringView>& lines, plInt32& ref_iLine, plStringView& ref_sSection)
{
  if (ref_iLine - 1 < (plInt32)lines.GetCount())
  {
    --ref_iLine;

    int iStepsBack = 0;

    while (ref_iLine >= 0)
    {
      if (lines[ref_iLine].StartsWith("//#ln"))
      {
        plStringView line = lines[ref_iLine];
        line.TrimWordStart("//#ln ");

        const char* szParsePos;
        plConversionUtils::StringToInt(line, ref_iLine, &szParsePos).AssertSuccess();

        line.SetStartPosition(szParsePos + 1);
        line.Trim("\"");

        ref_sSection = line;
        ref_iLine += iStepsBack - 1;
        break;
      }

      --ref_iLine;
      ++iStepsBack;
    }
  }
}
void plAngelScriptEngineSingleton::CompilerMessageCallback(const asSMessageInfo* msg)
{
  plDynamicArray<plStringView> lines;
  m_sCodeInCompilation.Split(true, lines, "\n");

  plInt32 iLine = msg->row;
  plStringView sSection = msg->section;

  FindCorrectSectionAndLine(lines, iLine, sSection);

  if (plDynamicArray<plScriptDiagnostic>* pCollector = GetDiagnosticCollector())
  {
    plScriptDiagnostic& diag = pCollector->ExpandAndGetRef();
    diag.m_sLanguage = "AngelScript";
    diag.m_Severity = msg->type == asMSGTYPE_ERROR ? plScriptDiagnosticSeverity::Error : msg->type == asMSGTYPE_WARNING ? plScriptDiagnosticSeverity::Warning : plScriptDiagnosticSeverity::Info;
    diag.m_Origin = plScriptDiagnosticOrigin::Compile;
    diag.m_sFile = sSection;
    diag.m_uiLine = iLine > 0 ? static_cast<plUInt32>(iLine) : 0;
    diag.m_uiColumn = msg->col > 0 ? static_cast<plUInt32>(msg->col) : 0;
    diag.m_sMessage = msg->message;
  }

  switch (msg->type)
  {
    case asMSGTYPE_ERROR:
      plLog::Error("{} ({}, {}) : {}", sSection, iLine, msg->col, msg->message);
      break;
    case asMSGTYPE_WARNING:
      plLog::Warning("{} ({}, {}) : {}", sSection, iLine, msg->col, msg->message);
      break;
    case asMSGTYPE_INFORMATION:
      plLog::Info("{} ({}, {}) : {}", sSection, iLine, msg->col, msg->message);
      break;
  }
}

asIScriptModule* plAngelScriptEngineSingleton::SetModuleCode(plStringView sModuleName, plStringView sCode, bool bAddExternalSection)
{
  PL_LOCK(m_CompilerMutex);

  m_sCodeInCompilation = sCode;

  plStringBuilder tmp;
  asIScriptModule* pModule = m_pEngine->GetModule(sModuleName.GetData(tmp), asGM_ALWAYS_CREATE);

  if (bAddExternalSection)
  {
    const char* szExternal = R"(
external shared class plAngelScriptClass;
)";

    pModule->AddScriptSection("External", szExternal);
  }

  pModule->AddScriptSection("Main", sCode.GetStartPointer(), sCode.GetElementCount());

  const int res = pModule->Build();
  switch (res)
  {
    case asBUILD_IN_PROGRESS:
      plLog::Error("AS: Another compilation is in progress.");
      break;

    case asINVALID_CONFIGURATION:
      plLog::Error("AS: Invalid Configuration.");
      break;

    case asINIT_GLOBAL_VARS_FAILED:
      plLog::Error("AS: Global Variable initialization failed.");
      break;

    case asNOT_SUPPORTED:
      plLog::Error("AS: Compiler support is disabled in the engine.");
      break;

    case asMODULE_IS_IN_USE:
      plLog::Error("AS: Module is in use.");
      break;

    case asERROR:
      break;
  }

  if (res < 0)
  {
    // TODO AngelScript: Forward compiler errors
    pModule->Discard();
    return nullptr;
  }

  return pModule;
}

plResult plAngelScriptEngineSingleton::PreprocessCode(plStringView sRefFilePath, plStringView sCode, plStringBuilder* out_pProcessedCode, plSet<plString>* out_pDependencies)
{
  plAsPreprocessor asPP;
  asPP.m_sRefFilePath = sRefFilePath;
  asPP.m_sMainCode = sCode;
  asPP.m_pDependencies = out_pDependencies;

  plStringBuilder fullCode;
  if (asPP.Process(fullCode).Failed())
    return PL_FAILURE;

  if (out_pProcessedCode)
  {
    *out_pProcessedCode = fullCode;
  }
  return PL_SUCCESS;
}

asIScriptModule* plAngelScriptEngineSingleton::CompileModule(plStringView sModuleName, plStringView sMainClass, plStringView sRefFilePath, plStringView sCode, plStringBuilder* out_pProcessedCode, plSet<plString>* out_pDependencies)
{
  plStringBuilder fullCode;
  if (PreprocessCode(sRefFilePath, sCode, &fullCode, out_pDependencies).Failed())
  {
    plLog::Error("Failed to pre-process AngelScript");
    return nullptr;
  }

  if (out_pProcessedCode)
  {
    *out_pProcessedCode = fullCode;
  }

  asIScriptModule* pModule = SetModuleCode(sModuleName, fullCode, true);

  if (pModule == nullptr)
    return nullptr;

  plStringBuilder tmp;
  const asITypeInfo* pClassType = pModule->GetTypeInfoByName(sMainClass.GetData(tmp));

  if (pClassType == nullptr)
  {
    plLog::Error("AngelScript code doesn't contain class '{}'", sMainClass);
    return nullptr;
  }

  if (ValidateModule(pModule).Failed())
  {
    return nullptr;
  }

  return pModule;
}


plResult plAngelScriptEngineSingleton::ValidateModule(asIScriptModule* pModule) const
{
  plResult res = PL_SUCCESS;

  for (plUInt32 i = 0; i < pModule->GetGlobalVarCount(); ++i)
  {
    const char* szName;
    int typeId;

    if (pModule->GetGlobalVar(i, &szName, nullptr, &typeId) == asSUCCESS)
    {
      if (const asITypeInfo* pInfo = pModule->GetEngine()->GetTypeInfoById(typeId))
      {
        if (IsTypeForbidden(pInfo))
        {
          plLog::Error("Global variable '{}' uses forbidden type '{}'", szName, pInfo->GetName());
          res = PL_FAILURE;
        }
      }
    }
  }

  for (plUInt32 i = 0; i < pModule->GetObjectTypeCount(); ++i)
  {
    const asITypeInfo* pType = pModule->GetObjectTypeByIndex(i);

    for (plUInt32 i2 = 0; i2 < pType->GetPropertyCount(); ++i2)
    {
      const char* szName;
      int typeId;

      pType->GetProperty(i2, &szName, &typeId);

      if (const asITypeInfo* pInfo = pModule->GetEngine()->GetTypeInfoById(typeId))
      {
        if (IsTypeForbidden(pInfo))
        {
          plLog::Error("Property '{}::{}' uses forbidden type '{}'", pType->GetName(), szName, pInfo->GetName());
          res = PL_FAILURE;
        }
      }
    }
  }

  return res;
}
