#pragma once

#include <GameEngineTest/GameEngineTestPCH.h>

#include "../TestClass/TestClass.h"

class plGameEngineTestApplication_AngelScript : public plGameEngineTestApplication
{
public:
  plGameEngineTestApplication_AngelScript();

  void SubTestBasicsSetup();
  plTestAppRun SubTestBasisExec(const char* szSubTestName);
  /// \brief Creates a module out of the given code and calls the function `void ExecuteTests()` in it.
  void RunTestScript(plStringView sScriptPath);
  void TestScriptExceptionCallback(asIScriptContext* pContext);

private:
  plStringBuilder m_sCode;
  plDynamicArray<plStringView> m_Lines;
};

class plGameEngineTestAngelScript : public plGameEngineTest
{
  using SUPER = plGameEngineTest;

public:
  virtual const char* GetTestName() const override;
  virtual plGameEngineTestApplication* CreateApplication() override;

  enum SubTests
  {
    Types,
    Strings,
    Arrays,
    EntryPoints,
    World,
    Messaging,
    GameObject,
    Physics,
    Misc,
  };

private:
  virtual void SetupSubTests() override;
  virtual plResult InitializeSubTest(plInt32 iIdentifier) override;
  virtual plTestAppRun RunSubTest(plInt32 iIdentifier, plUInt32 uiInvocationCount) override;

  plGameEngineTestApplication_AngelScript* m_pOwnApplication = nullptr;
};
