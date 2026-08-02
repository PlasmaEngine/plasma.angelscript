#pragma once

#include <AngelScriptPlugin/AngelScriptPluginDLL.h>
#include <Core/Scripting/Bindings/ScriptBindingRegistry.h>
#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Memory/CommonAllocators.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/UniquePtr.h>

class asIScriptEngine;
class asIScriptModule;
class asIStringFactory;
struct asSMessageInfo;
class plAsStringFactory;

#if PL_ENABLED(PL_COMPILE_FOR_DEVELOPMENT)
using plAsAllocatorType = plAllocatorWithPolicy<plAllocPolicyHeap, plAllocatorTrackingMode::AllocationStats>;
#else
using plAsAllocatorType = plAllocatorWithPolicy<plAllocPolicyHeap, plAllocatorTrackingMode::Nothing>;
#endif

class PL_ANGELSCRIPTPLUGIN_DLL plAngelScriptEngineSingleton
{
  PL_DECLARE_SINGLETON(plAngelScriptEngineSingleton);

public:
  static plResult PreprocessCode(plStringView sRefFilePath, plStringView sCode, plStringBuilder* out_pProcessedCode, plSet<plString>* out_pDependencies);
  static void FindCorrectSectionAndLine(const plDynamicArray<plStringView>& lines, plInt32& ref_iLine, plStringView& ref_sSection);

public:
  plAngelScriptEngineSingleton();
  ~plAngelScriptEngineSingleton();

  asIScriptEngine* GetEngine() const { return m_pEngine; }

  asIScriptModule* SetModuleCode(plStringView sModuleName, plStringView sCode, bool bAddExternalSection);
  asIScriptModule* CompileModule(plStringView sModuleName, plStringView sMainClass, plStringView sRefFilePath, plStringView sCode, plStringBuilder* out_pProcessedCode, plSet<plString>* out_pDependencies);
  plResult ValidateModule(asIScriptModule* pModule) const;

  const plSet<plString>& GetNotRegistered() const { return m_NotRegistered; }

private:
  void AddForbiddenType(const char* szTypeName);
  bool IsTypeForbidden(const asITypeInfo* pType) const;

  void CompilerMessageCallback(const asSMessageInfo* msg);
  void ExceptionCallback(asIScriptContext* pContext);

  void RegisterStandardTypes();
  void Register_RTTI();
  void Register_Vec2();
  void Register_Vec3();
  void Register_Vec4();
  void Register_Angle();
  void Register_Quat();
  void Register_Transform();
  void Register_GameObject();
  void Register_Time();
  void Register_Mat3();
  void Register_Mat4();
  void Register_World();
  void Register_Clock();
  void Register_String();
  void Register_StringView();
  void Register_StringBuilder();
  void Register_TempHashedString();
  void Register_HashedString();
  void Register_Color();
  void Register_ColorGammaUB();
  void Register_Random();
  void Register_Math();
  void Register_Spatial();

  void Register_plAngelScriptClass();
  void Register_GlobalReflectedFunctions();
  void Register_ReflectedType(const plRTTI* pBaseType, bool bCreatable);
  void Register_ReflectedTypes();
  void RegisterTypeFunctions(const char* szTypeName, const plRTTI* pRtti, bool bIsInherited);
  void RegisterSingleGenericFunction(const char* szFuncName, const char* szTypeName, const plAbstractFunctionProperty* const pFunc, const plScriptableFunctionAttribute* pFuncAttr, bool bIsInherited, const plRTTI* pReturnType);
  void RegisterGenericFunction(const char* szTypeName, const plAbstractFunctionProperty* const pFunc, const plScriptableFunctionAttribute* pFuncAttr, bool bIsInherited);
  bool AppendType(plStringBuilder& decl, const plRTTI* pRtti, const plScriptableFunctionAttribute* pFuncAttr, plUInt32 uiArg);
  bool AppendFuncArgs(plStringBuilder& decl, const plAbstractFunctionProperty* pFunc, const plScriptableFunctionAttribute* pFuncAttr, plUInt32 uiArg);
  void Register_ExtraComponentFuncs();


  template <typename T>
  void RegisterPodValueType(asUINT additonalFlags = 0)
  {
    const plRTTI* pRtti = plGetStaticRTTI<T>();
    int typeId = m_pEngine->RegisterObjectType(pRtti->GetTypeName().GetStartPointer(), sizeof(T), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<T>() | additonalFlags);
    AS_CHECK(typeId);

    m_pEngine->GetTypeInfoById(typeId)->SetUserData((void*)pRtti, plAsUserData::RttiPtr);
  }

  template <typename T>
  void RegisterNonPodValueType()
  {
    const plRTTI* pRtti = plGetStaticRTTI<T>();
    int typeId = m_pEngine->RegisterObjectType(pRtti->GetTypeName().GetStartPointer(), sizeof(T), asOBJ_VALUE | asGetTypeTraits<T>());
    AS_CHECK(typeId);

    m_pEngine->GetTypeInfoById(typeId)->SetUserData((void*)pRtti, plAsUserData::RttiPtr);
  }

  template <typename T>
  void RegisterRefType()
  {
    const plRTTI* pRtti = plGetStaticRTTI<T>();
    int typeId = m_pEngine->RegisterObjectType(pRtti->GetTypeName().GetStartPointer(), 0, asOBJ_REF | asOBJ_NOCOUNT);
    AS_CHECK(typeId);

    m_pEngine->GetTypeInfoById(typeId)->SetUserData((void*)pRtti, plAsUserData::RttiPtr);

    AddForbiddenType(pRtti->GetTypeName().GetStartPointer());

    m_WhitelistedRefTypes.Insert(pRtti->GetTypeName());
  }

  asIScriptEngine* m_pEngine = nullptr;

  plSet<plString> m_WhitelistedRefTypes;

  plHybridArray<const asITypeInfo*, 16> m_ForbiddenTypes;

  plAsStringFactory* m_pStringFactory = nullptr;

  plSet<plString> m_NotRegistered;

  plScriptBindingSnapshot m_BindingSnapshot;

  plMutex m_CompilerMutex;
  plStringView m_sCodeInCompilation;
};
