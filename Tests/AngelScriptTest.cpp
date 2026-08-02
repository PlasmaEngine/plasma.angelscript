#include <GameEngineTest/GameEngineTestPCH.h>

#ifdef BUILDSYSTEM_ENABLE_ANGELSCRIPT_SUPPORT

#  include <AngelScript/include/angelscript.h>
#  include <AngelScriptPlugin/Runtime/AsEngineSingleton.h>
#  include <Core/Messages/CommonMessages.h>
#  include <Core/WorldSerializer/WorldReader.h>
#  include <Foundation/CodeUtils/Preprocessor.h>
#  include <Foundation/IO/FileSystem/FileReader.h>

#  include "AngelScriptTest.h"

static plGameEngineTestAngelScript s_GameEngineTestAngelScript;

const char* plGameEngineTestAngelScript::GetTestName() const
{
  return "AngelScript Tests";
}

plGameEngineTestApplication* plGameEngineTestAngelScript::CreateApplication()
{
  m_pOwnApplication = PL_DEFAULT_NEW(plGameEngineTestApplication_AngelScript);
  return m_pOwnApplication;
}

void plGameEngineTestAngelScript::SetupSubTests()
{
  AddSubTest("Types", SubTests::Types);
  AddSubTest("Strings", SubTests::Strings);
  AddSubTest("Arrays", SubTests::Arrays);
  AddSubTest("EntryPoints", SubTests::EntryPoints);
  AddSubTest("World", SubTests::World);
  AddSubTest("Messaging", SubTests::Messaging);
  AddSubTest("GameObject", SubTests::GameObject);
  AddSubTest("Physics", SubTests::Physics);
  AddSubTest("Misc", SubTests::Misc);
}

plResult plGameEngineTestAngelScript::InitializeSubTest(plInt32 iIdentifier)
{
  m_pOwnApplication->SubTestBasicsSetup();
  return PL_SUCCESS;
}

plTestAppRun plGameEngineTestAngelScript::RunSubTest(plInt32 iIdentifier, plUInt32 uiInvocationCount)
{
  switch (iIdentifier)
  {
    case SubTests::Types:
      m_pOwnApplication->RunTestScript("Tests/Types/TypesTest.as");
      return plTestAppRun::Quit;
    case SubTests::Strings:
      m_pOwnApplication->RunTestScript("Tests/Types/StringsTest.as");
      return plTestAppRun::Quit;
    case SubTests::Arrays:
      m_pOwnApplication->RunTestScript("Tests/Types/ArraysTest.as");
      return plTestAppRun::Quit;
    default:
      return m_pOwnApplication->SubTestBasisExec(GetSubTestName(iIdentifier));
  }
}

//////////////////////////////////////////////////////////////////////////

plGameEngineTestApplication_AngelScript::plGameEngineTestApplication_AngelScript()
  : plGameEngineTestApplication("AngelScript")
{
}

void plGameEngineTestApplication_AngelScript::SubTestBasicsSetup()
{
  LoadScene("AngelScript/AssetCache/Common/Scenes/Main.plBinScene").IgnoreResult();
}

plTestAppRun plGameEngineTestApplication_AngelScript::SubTestBasisExec(const char* szSubTestName)
{
  Run();
  if (WasQuitRequested())
    return plTestAppRun::Quit;

  PL_LOCK(m_pWorld->GetWriteMarker());

  plGameObject* pTests = nullptr;
  if (m_pWorld->TryGetObjectWithGlobalKey("Tests", pTests) == false)
  {
    PL_TEST_FAILURE("Failed to retrieve AngelScript Tests-Object", "");
    return plTestAppRun::Quit;
  }

  const plStringBuilder sMsg(szSubTestName, "Test");

  plMsgGenericEvent msg;
  msg.m_sMessage.Assign(sMsg);
  pTests->SendMessageRecursive(msg);

  if (msg.m_sMessage == plTempHashedString("repeat"))
    return plTestAppRun::Continue;

  PL_TEST_STRING(msg.m_sMessage, "done");

  return plTestAppRun::Quit;
}

void plGameEngineTestApplication_AngelScript::RunTestScript(plStringView sScriptPath)
{
  // Load and process test script
  {
    plStringBuilder sTestCode;
    plStringBuilder sProcessedCode;
    {
      plFileReader read;
      if (read.Open(sScriptPath).Failed())
      {
        plLog::Error("Failed to open file '{}'.", sScriptPath);
        return;
      }
      sTestCode.ReadAll(read);
    }
    if (plAngelScriptEngineSingleton::PreprocessCode(sScriptPath, sTestCode, &sProcessedCode, nullptr).Failed())
    {
      plLog::Error("Failed to preprocess code '{}'.", sScriptPath);
      return;
    }
    m_sCode = sProcessedCode;
    m_sCode.Split(true, m_Lines, "\n");
  }

  PL_LOCK(m_pWorld->GetWriteMarker());

  auto pAsEngine = plAngelScriptEngineSingleton::GetSingleton();
  asIScriptModule* pModule = pAsEngine->SetModuleCode("ScriptTest", m_sCode, true);
  if (pModule == nullptr)
  {
    plLog::Error("Failed to create AngelScript module.");
    return;
  }
  asIScriptContext* m_pContext = pAsEngine->GetEngine()->CreateContext();
  PL_SCOPE_EXIT(m_pContext->Release(););
  AS_CHECK(m_pContext->SetExceptionCallback(asMETHOD(plGameEngineTestApplication_AngelScript, TestScriptExceptionCallback), this, asCALL_THISCALL));

  asIScriptFunction* func = pModule->GetFunctionByName("ExecuteTests");
  m_pContext->Prepare(func);

  PL_TEST_INT(m_pContext->Execute(), asEXECUTION_FINISHED);
}

void plGameEngineTestApplication_AngelScript::TestScriptExceptionCallback(asIScriptContext* pContext)
{
  plLog::Error("AS Exception '{}'", pContext->GetExceptionString());

  const plUInt32 uiNumLevels = pContext->GetCallstackSize();
  for (plUInt32 i = 0; i < uiNumLevels; ++i)
  {
    const char* szSection = nullptr;
    const plInt32 iOriginalLine = pContext->GetLineNumber(i, nullptr, &szSection);

    plInt32 iLine = iOriginalLine;
    plStringView sSection = szSection;
    plAngelScriptEngineSingleton::FindCorrectSectionAndLine(m_Lines, iLine, sSection);

    plStringBuilder line("  ");
    if (asIScriptFunction* pFunc = pContext->GetFunction(i))
    {
      if (!plStringUtils::IsNullOrEmpty(pFunc->GetNamespace()))
      {
        line.Append(pFunc->GetNamespace(), "::");
      }

      if (!plStringUtils::IsNullOrEmpty(pFunc->GetObjectName()))
      {
        line.Append(pFunc->GetObjectName(), "::");
      }

      line.AppendFormat("{}() [{}: {}] -> {}", pFunc->GetName(), sSection, iLine, m_Lines[iOriginalLine - 1]);

      plLog::Error(line);
    }
    else
    {
      line.AppendFormat("<nested call> [{}: {}] -> {}", sSection, iLine, m_Lines[iOriginalLine - 1]);
    }
  }
}

#endif
