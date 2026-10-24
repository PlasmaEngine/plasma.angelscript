#include <EditorPluginAngelScript/EditorPluginAngelScriptPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Runtime/AsEngineSingleton.h>
#include <EditorFramework/EditorApp/EditorApp.h>
#include <EditorFramework/ProjectChecks/ProjectChecks.h>
#include <EditorPluginAngelScript/Tests/AngelScriptTestRunner.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/OSFile.h>
#include <ToolsFoundation/Project/ToolsProject.h>

namespace
{
  struct TestFile
  {
    plString m_sAbsolute;
    plString m_sRelative;
    plString m_sCode;
  };

  bool IsTestCode(plStringView sCode)
  {
    return sCode.FindSubString("void ExecuteTests(") != nullptr || sCode.FindSubString("void Test") != nullptr;
  }

  void FindTestFiles(plDynamicArray<TestFile>& out_files)
  {
    if (!plToolsProject::IsProjectOpen())
      return;

    plFileSystemIterator it;
    for (it.StartSearch(plToolsProject::GetSingleton()->GetProjectDirectory(), plFileSystemIteratorFlags::ReportFilesAndFoldersRecursive); it.IsValid(); it.Next())
    {
      const plStringView sName = it.GetStats().m_sName;
      if (it.GetStats().m_bIsDirectory)
      {
        // Generated and tool folders hold no tests.
        if (sName.StartsWith(".") || sName == "AssetCache" || sName == "bin" || sName == "obj")
          it.SkipFolder();
        continue;
      }

      if (!plPathUtils::HasExtension(sName, "as"))
        continue;

      plStringBuilder sAbsolute = it.GetCurrentPath();
      sAbsolute.AppendPath(sName);

      plFileReader file;
      if (file.Open(sAbsolute).Failed())
        continue;

      plStringBuilder sCode;
      sCode.ReadAll(file);
      if (!IsTestCode(sCode))
        continue;

      plStringBuilder sRelative = sAbsolute;
      plEditorApp::GetSingleton()->MakePathDataDirectoryRelative(sRelative);

      TestFile& test = out_files.ExpandAndGetRef();
      test.m_sAbsolute = sAbsolute;
      test.m_sRelative = sRelative;
      test.m_sCode = sCode;
    }
  }

  /// Where an exception happened, mapped back from the pre-processed module to the test's own files.
  void GetExceptionPlace(asIScriptContext* pContext, asIScriptModule* pModule, const TestFile& test, plString& out_sFile, plUInt32& out_uiLine)
  {
    out_sFile = test.m_sRelative;
    out_uiLine = 0;

    const char* szSection = nullptr;
    const int iRow = pContext->GetExceptionLineNumber(nullptr, &szSection);
    if (iRow <= 0)
      return;

    plStringView sFile;
    plInt32 iLine = 0;
    if (plAngelScriptEngineSingleton::GetSingleton()->MapLocation(pModule, iRow, sFile, iLine).Succeeded())
    {
      plStringBuilder sRelative = sFile;
      plEditorApp::GetSingleton()->MakePathDataDirectoryRelative(sRelative);
      out_sFile = sRelative;
      out_uiLine = static_cast<plUInt32>(iLine);
    }
    else
    {
      out_uiLine = static_cast<plUInt32>(iRow);
    }
  }

  void RunTestFile(plProjectChecks::Context& ref_context, const TestFile& test, plUInt32& inout_uiPassed, plUInt32& inout_uiFailed)
  {
    plAngelScriptEngineSingleton* pAs = plAngelScriptEngineSingleton::GetSingleton();

    plStringBuilder sProcessed;
    plDynamicArray<plScriptDiagnostic> diagnostics;
    plAngelScriptEngineSingleton::SetDiagnosticCollector(&diagnostics);

    asIScriptModule* pModule = nullptr;
    if (plAngelScriptEngineSingleton::PreprocessCode(test.m_sAbsolute, test.m_sCode, &sProcessed, nullptr).Succeeded())
    {
      pModule = pAs->SetModuleCode("ProjectCheckTest", sProcessed, true);
      if (pModule != nullptr)
        pAs->RegisterSourceMap(pModule, sProcessed);
    }

    plAngelScriptEngineSingleton::SetDiagnosticCollector(nullptr);

    if (pModule == nullptr)
    {
      ++inout_uiFailed;
      bool bReported = false;
      for (const plScriptDiagnostic& diag : diagnostics)
      {
        if (diag.m_Severity != plScriptDiagnosticSeverity::Error)
          continue;
        ref_context.Error(diag.m_sMessage, test.m_sRelative, diag.m_sFile.IsEmpty() ? test.m_sRelative.GetView() : diag.m_sFile.GetView(), diag.m_uiLine);
        ref_context.m_pProblems->PeekBack().m_sLanguage = "AngelScript";
        bReported = true;
      }
      if (!bReported)
        ref_context.Error("The test file does not compile.", test.m_sRelative, test.m_sRelative);
      return;
    }

    PL_SCOPE_EXIT(pAs->UnregisterSourceMap(pModule); pModule->Discard());

    asIScriptContext* pContext = pAs->GetEngine()->CreateContext();
    PL_SCOPE_EXIT(pContext->Release());

    for (asUINT f = 0; f < pModule->GetFunctionCount(); ++f)
    {
      asIScriptFunction* pFunction = pModule->GetFunctionByIndex(f);
      const plStringView sName = pFunction->GetName();
      if (pFunction->GetParamCount() != 0 || (sName != "ExecuteTests" && !sName.StartsWith("Test")))
        continue;

      plStringBuilder sTestName(plPathUtils::GetFileName(test.m_sRelative), " \xC2\xB7 ", sName);

      pContext->Prepare(pFunction);
      const int iResult = pContext->Execute();

      if (iResult == asEXECUTION_FINISHED)
      {
        ++inout_uiPassed;
        ref_context.AddProblem(plScriptDiagnosticSeverity::Info, sTestName, test.m_sRelative, test.m_sRelative);
        ref_context.m_pProblems->PeekBack().m_sLanguage = "AngelScript";
        continue;
      }

      ++inout_uiFailed;

      plString sFile;
      plUInt32 uiLine = 0;
      plStringBuilder sMessage(sTestName, ": ");
      if (iResult == asEXECUTION_EXCEPTION)
      {
        GetExceptionPlace(pContext, pModule, test, sFile, uiLine);
        sMessage.Append(pContext->GetExceptionString());
      }
      else
      {
        sFile = test.m_sRelative;
        sMessage.Append("did not finish");
      }

      ref_context.Error(sMessage, test.m_sRelative, sFile, uiLine);
      ref_context.m_pProblems->PeekBack().m_sLanguage = "AngelScript";
    }
  }

  void RunAngelScriptTests(plProjectChecks::Context& ref_context)
  {
    if (plAngelScriptEngineSingleton::GetOrCreate() == nullptr)
    {
      ref_context.Error("The AngelScript engine did not start, so no test ran", "");
      return;
    }

    plDynamicArray<TestFile> files;
    FindTestFiles(files);

    plUInt32 uiPassed = 0;
    plUInt32 uiFailed = 0;
    for (const TestFile& test : files)
      RunTestFile(ref_context, test, uiPassed, uiFailed);

    ref_context.m_sSummary.SetFormat("AngelScript: {} tests in {} files, {} failed", uiPassed + uiFailed, files.GetCount(), uiFailed);
  }
} // namespace

void plRegisterAngelScriptTestRunner()
{
  if (plProjectChecks* pChecks = plProjectChecks::GetSingleton())
    pChecks->RegisterTestRunner("AngelScript", plProjectChecks::Runner(&RunAngelScriptTests));
}

void plUnregisterAngelScriptTestRunner()
{
  if (plProjectChecks* pChecks = plProjectChecks::GetSingleton())
    pChecks->UnregisterTestRunner("AngelScript");
}
