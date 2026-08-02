#pragma once

#include <AngelScriptPlugin/AngelScriptPluginDLL.h>

#include <Foundation/Basics.h>
#include <Foundation/Types/VariantType.h>

class asIScriptEngine;
class plVariant;
class asIScriptGeneric;
class plAbstractFunctionProperty;
class asIScriptModule;
class asIScriptFunction;
struct plScriptBindingSnapshot;

struct PL_ANGELSCRIPTPLUGIN_DLL plAsInfos
{
  plSet<plString> m_Types;
  plSet<plString> m_Namespaces;
  plSet<plString> m_GlobalFunctions;
  plSet<plString> m_Methods;
  plSet<plString> m_AllDeclarations;
  plSet<plString> m_Properties;
  plSet<plString> m_EnumValues;
};

class PL_ANGELSCRIPTPLUGIN_DLL plAngelScriptUtils
{
public:
  static void SetThreadLocalWorld(plWorld* pWorld);
  static plWorld* GetThreadLocalWorld();

  static void SaveByteCode(asIScriptModule* pModule, plDynamicArray<plUInt8>& out_byteCode);

  static const char* GetAsTypeName(asIScriptEngine* pEngine, int iAsTypeID);

  static plString GetNiceFunctionDeclaration(const asIScriptFunction* pFunc, bool bIncludeObjectName = false, bool bIncludeNamespace = false);

  static asIScriptModule* LoadFromByteCode(asIScriptEngine* pEngine, plStringView sModuleName, plArrayPtr<plUInt8> byteCode);

  static const plRTTI* MapToRTTI(int iAsTypeID, asIScriptEngine* pEngine);

  static plResult WriteToAsTypeAtLocation(asIScriptEngine* pEngine, int iAsTypeID, void* pMemoryLocation, const plVariant& value);
  static plResult ReadFromAsTypeAtLocation(asIScriptEngine* pEngine, int iAsTypeID, void* pMemoryLocation, plVariant& out_value);

  static const char* VariantTypeToString(plVariantType::Enum type);

  static plString DefaultValueToString(const plVariant& value, plVariantType::Enum expectedType);

  static void RetrieveArg(asIScriptGeneric* pGen, plUInt32 uiRealArg, plInt32& ref_iSkippedArg, const plAbstractFunctionProperty* pAbstractFuncProp, plVariant& out_arg);

  static void RetrieveVarArgs(asIScriptGeneric* pGen, plUInt32 uiStartArg, const plAbstractFunctionProperty* pAbstractFuncProp, plVariant& out_arg);

  static void MakeGenericFunctionCall(asIScriptGeneric* pGen);

  static void DefaultConstructInPlace(void* pPtr, const plRTTI* pRtti);

  static void RetrieveAsInfos(asIScriptEngine* pEngine, plAsInfos& out_infos);

  static void GenerateAsPredefinedFile(asIScriptEngine* pEngine, plStringBuilder& out_sContent);

  static plString RegisterEnumType(asIScriptEngine* pEngine, const plRTTI* pEnumType);

  static void RegisterTypeProperties(
    asIScriptEngine* pEngine,
    const char* szTypeName,
    const plScriptBindingSnapshot& bindingSnapshot,
    const plRTTI* pRtti,
    bool bIsInherited);
};
