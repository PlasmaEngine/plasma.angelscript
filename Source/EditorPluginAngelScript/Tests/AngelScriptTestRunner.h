#pragma once

/// Registers the AngelScript runner of the project checks' ScriptTests kind.
///
/// A test file is any `.as` file of the project that defines `void ExecuteTests()` or functions named `Test...` without
/// parameters. Each of those functions is one test. They run in the editor with the AngelScript engine and no world, so
/// they test script logic; a failed assertion is an exception (see TestFramework.as in the package tests).
void plRegisterAngelScriptTestRunner();
void plUnregisterAngelScriptTestRunner();
