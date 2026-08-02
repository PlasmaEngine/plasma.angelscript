#include <AngelScriptPlugin/AngelScriptPluginPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Runtime/AsEngineSingleton.h>
#include <AngelScriptPlugin/Utils/AngelScriptUtils.h>
#include <Foundation/Reflection/ReflectionUtils.h>

bool plAngelScriptEngineSingleton::AppendType(plStringBuilder& decl, const plRTTI* pRtti, const plScriptableFunctionAttribute* pFuncAttr, plUInt32 uiArg)
{
  const bool bIsReturnValue = uiArg == plInvalidIndex;
  const auto argType = pFuncAttr ? pFuncAttr->GetArgumentType(uiArg) : plScriptableFunctionAttribute::ArgType::In;

  if (pRtti == nullptr || pRtti == plGetStaticRTTI<plVariantArray>())
  {
    decl.Append("void");
    return bIsReturnValue;
  }

  if (argType == plScriptableFunctionAttribute::ArgType::Inout)
  {
    // not yet supported for most types
    return false;
  }

  if (const char* szTypeName = plAngelScriptUtils::VariantTypeToString(pRtti->GetVariantType()); szTypeName != nullptr)
  {
    decl.Append(szTypeName);

    if (argType == plScriptableFunctionAttribute::ArgType::Out)
    {
      decl.Append("& out");
    }

    return true;
  }

  if (pRtti->GetTypeFlags().IsAnySet(plTypeFlags::IsEnum | plTypeFlags::Bitflags))
  {
    decl.Append(plAngelScriptUtils::RegisterEnumType(m_pEngine, pRtti));

    if (argType == plScriptableFunctionAttribute::ArgType::Out)
    {
      decl.Append("& out");
    }
    return true;
  }

  if (!bIsReturnValue)
  {
    if (pRtti == plGetStaticRTTI<plVariant>())
    {
      decl.Append("?& in");
      return true;
    }

    if (pRtti == plGetStaticRTTI<plWorld>())
    {
      // skip
      return true;
    }
  }

  if (pRtti == plGetStaticRTTI<plGameObjectHandle>() || pRtti == plGetStaticRTTI<plComponentHandle>())
  {
    decl.Append(pRtti->GetTypeName());

    if (argType == plScriptableFunctionAttribute::ArgType::Out)
    {
      decl.Append("& out");
    }

    return true;
  }

  if (m_WhitelistedRefTypes.Contains(pRtti->GetTypeName()))
  {
    decl.Append(pRtti->GetTypeName(), "@");
    return true;
  }

  decl.Append(pRtti->GetTypeName());
  m_NotRegistered.Insert(decl);
  return false;
}

bool plAngelScriptEngineSingleton::AppendFuncArgs(plStringBuilder& decl, const plAbstractFunctionProperty* pFunc, const plScriptableFunctionAttribute* pFuncAttr, plUInt32 uiArg)
{
  if (uiArg > 12)
  {
    PL_ASSERT_DEBUG(false, "Too many function arguments");
    return false;
  }

  if (uiArg > 0 && !decl.EndsWith("("))
  {
    decl.Append(", ");
  }

  return AppendType(decl, pFunc->GetArgumentType(uiArg), pFuncAttr, uiArg);
}

void plAngelScriptEngineSingleton::Register_GlobalReflectedFunctions()
{
  PL_LOG_BLOCK("Register_GlobalReflectedFunctions");

  plDynamicArray<const plScriptBindingTypeDesc*> types;
  m_BindingSnapshot.GetTypesInReflectionOrder(types);
  for (const plScriptBindingTypeDesc* pType : types)
  {
    const plRTTI* pRtti = pType->m_pRtti;
    if (pRtti->GetParentType() != nullptr && pRtti->GetParentType() != plGetStaticRTTI<plNoBase>())
      continue;

    plDynamicArray<const plScriptBindingFunctionDesc*> functions;
    m_BindingSnapshot.GetFunctions(pRtti, functions);
    for (const plScriptBindingFunctionDesc* pFunction : functions)
    {
      if (pFunction->m_FunctionType != plFunctionType::StaticMember)
        continue;

      const plAbstractFunctionProperty* pFunc = pFunction->m_pFunction;
      const plScriptableFunctionAttribute* pFuncAttr = pFunc->GetAttributeByType<plScriptableFunctionAttribute>();
      PL_ASSERT_DEV(pFuncAttr != nullptr, "Shared binding descriptor lost its scriptable function attribute.");

      RegisterGenericFunction(pType->m_sNativeName, pFunc, pFuncAttr, false);
    }
  }
}

static void CastToBase(asIScriptGeneric* pGen)
{
  int derivedTypeId = pGen->GetObjectTypeId();
  auto derivedTypeInfo = pGen->GetEngine()->GetTypeInfoById(derivedTypeId);
  const plRTTI* pDerivedRtti = (const plRTTI*)derivedTypeInfo->GetUserData(plAsUserData::RttiPtr);

  const plRTTI* pBaseRtti = (const plRTTI*)pGen->GetAuxiliary();

  if (pDerivedRtti != nullptr && pBaseRtti != nullptr)
  {
    if (pDerivedRtti->IsDerivedFrom(pBaseRtti))
    {
      pGen->SetReturnObject(pGen->GetObject());
      return;
    }
  }

  pGen->SetReturnObject(nullptr);
}

static void CastToDerived(asIScriptGeneric* pGen)
{
  int baseTypeId = pGen->GetObjectTypeId();
  auto baseTypeInfo = pGen->GetEngine()->GetTypeInfoById(baseTypeId);
  const plRTTI* pBaseRtti = (const plRTTI*)baseTypeInfo->GetUserData(plAsUserData::RttiPtr);

  const plRTTI* pDerivedRtti = (const plRTTI*)pGen->GetAuxiliary();

  if (pBaseRtti != nullptr && pDerivedRtti != nullptr)
  {
    if (pDerivedRtti->IsDerivedFrom(pBaseRtti))
    {
      pGen->SetReturnObject(pGen->GetObject());
      return;
    }
  }

  pGen->SetReturnObject(nullptr);
}

struct RefInstance
{
  plUInt32 m_uiRefCount = 1;
  const plRTTI* m_pRtti = nullptr;
};

static plMutex s_RefCountMutex;
static plMap<void*, RefInstance> s_RefCounts;

static void* plRtti_Create(const plRTTI* pRtti)
{
  auto inst = pRtti->GetAllocator()->Allocate<plReflectedClass>();

  PL_LOCK(s_RefCountMutex);
  auto& ref = s_RefCounts[inst.m_pInstance];
  ref.m_pRtti = pRtti;

  return inst.m_pInstance;
}

static void plRtti_AddRef(void* pInstance)
{
  PL_LOCK(s_RefCountMutex);
  ++s_RefCounts[pInstance].m_uiRefCount;
}

static void plRtti_Release(void* pInstance)
{
  PL_LOCK(s_RefCountMutex);
  auto it = s_RefCounts.Find(pInstance);
  RefInstance& ri = it.Value();
  if (--ri.m_uiRefCount == 0)
  {
    ri.m_pRtti->GetAllocator()->Deallocate(pInstance);
    s_RefCounts.Remove(it);
  }
}


void plAngelScriptEngineSingleton::Register_ReflectedType(const plRTTI* pBaseType, bool bCreatable)
{
  PL_LOG_BLOCK("Register_ReflectedType", pBaseType->GetTypeName());

  plDynamicArray<const plScriptBindingTypeDesc*> types;
  m_BindingSnapshot.GetTypesInReflectionOrder(types);

  // first register the type
  for (const plScriptBindingTypeDesc* pType : types)
  {
    const plRTTI* pRtti = pType->m_pRtti;
    if (!pRtti->IsDerivedFrom(pBaseType))
      continue;

    if (pType->m_bHidden || pType->m_bExcludedFromScript)
      continue;

    plStringBuilder typeName = pType->m_sNativeName;
    auto pTypeInfo = m_pEngine->GetTypeInfoByName(typeName);

    m_WhitelistedRefTypes.Insert(typeName);

    if (pTypeInfo == nullptr)
    {
      if (bCreatable)
      {
        const int typeId = m_pEngine->RegisterObjectType(typeName, 0, asOBJ_REF);
        AS_CHECK(typeId);
        pTypeInfo = m_pEngine->GetTypeInfoById(typeId);

        if (pType->m_bCanAllocate)
        {
          const plStringBuilder sFactoryOp(typeName, "@ f()");
          m_pEngine->RegisterObjectBehaviour(typeName, asBEHAVE_FACTORY, sFactoryOp, asFUNCTION(plRtti_Create), asCALL_CDECL_OBJLAST, (void*)pRtti);
          m_pEngine->RegisterObjectBehaviour(typeName, asBEHAVE_ADDREF, "void f()", asFUNCTION(plRtti_AddRef), asCALL_CDECL_OBJLAST, (void*)pRtti);
          m_pEngine->RegisterObjectBehaviour(typeName, asBEHAVE_RELEASE, "void f()", asFUNCTION(plRtti_Release), asCALL_CDECL_OBJLAST, (void*)pRtti);
        }
      }
      else
      {

        const int typeId = m_pEngine->RegisterObjectType(typeName, 0, asOBJ_REF | asOBJ_NOCOUNT);
        AS_CHECK(typeId);

        pTypeInfo = m_pEngine->GetTypeInfoById(typeId);
      }
    }

    pTypeInfo->SetUserData((void*)pRtti, plAsUserData::RttiPtr);

    AddForbiddenType(typeName);

    RegisterTypeFunctions(typeName, pRtti, false);
    plAngelScriptUtils::RegisterTypeProperties(m_pEngine, typeName, m_BindingSnapshot, pRtti, false);
  }

  // then register the type hierarchy
  for (const plScriptBindingTypeDesc* pType : types)
  {
    const plRTTI* pRtti = pType->m_pRtti;
    if (!pRtti->IsDerivedFrom(pBaseType) || pRtti == pBaseType)
      continue;

    if (pType->m_bHidden || pType->m_bExcludedFromScript)
      continue;

    const plStringBuilder typeName = pType->m_sNativeName;

    const plRTTI* pParentRtti = pRtti->GetParentType();

    plStringBuilder parentName, castOp;

    while (pParentRtti)
    {
      parentName = pParentRtti->GetTypeName();
      castOp.Set(parentName, "@ opImplCast()");

      AS_CHECK(m_pEngine->RegisterObjectMethod(typeName, castOp, asFUNCTION(CastToBase), asCALL_GENERIC, (void*)pParentRtti));

      castOp.Set(typeName, "@ opCast()");
      AS_CHECK(m_pEngine->RegisterObjectMethod(parentName, castOp, asFUNCTION(CastToDerived), asCALL_GENERIC, (void*)pRtti));

      if (pParentRtti == pBaseType)
        break;

      pParentRtti = pParentRtti->GetParentType();
    }
  }
}

void plAngelScriptEngineSingleton::RegisterTypeFunctions(const char* szTypeName, const plRTTI* pRtti, bool bIsInherited)
{
  if (pRtti == nullptr || pRtti == plGetStaticRTTI<plReflectedClass>())
    return;

  plDynamicArray<const plScriptBindingFunctionDesc*> functions;
  m_BindingSnapshot.GetFunctions(pRtti, functions);
  for (const plScriptBindingFunctionDesc* pFunction : functions)
  {
    const plAbstractFunctionProperty* pFunc = pFunction->m_pFunction;
    const plScriptableFunctionAttribute* pFuncAttr = pFunc->GetAttributeByType<plScriptableFunctionAttribute>();
    PL_ASSERT_DEV(pFuncAttr != nullptr, "Shared binding descriptor lost its scriptable function attribute.");

    RegisterGenericFunction(szTypeName, pFunc, pFuncAttr, bIsInherited);
  }

  RegisterTypeFunctions(szTypeName, pRtti->GetParentType(), true);
}

static void CollectFunctionArgumentAttributes(const plAbstractFunctionProperty* pFuncProp, plDynamicArray<const plFunctionArgumentAttributes*>& out_attributes)
{
  for (auto pAttr : pFuncProp->GetAttributes())
  {
    if (auto pFuncArgAttr = plDynamicCast<const plFunctionArgumentAttributes*>(pAttr))
    {
      plUInt32 uiArgIndex = pFuncArgAttr->GetArgumentIndex();
      out_attributes.EnsureCount(uiArgIndex + 1);
      PL_ASSERT_DEV(out_attributes[uiArgIndex] == nullptr, "Multiple argument attributes for argument {} of '{}'", uiArgIndex, pFuncProp->GetPropertyName());
      PL_ASSERT_DEV(uiArgIndex < pFuncProp->GetArgumentCount(), "Function argument attribute for argument {} of '{}' which only has {} arguments.", uiArgIndex, pFuncProp->GetPropertyName(), pFuncProp->GetArgumentCount());
      out_attributes[uiArgIndex] = pFuncArgAttr;
    }
  }
}

static bool ExistsGlobalFunc(asIScriptEngine* pEngine, const char* szNamespace, const char* szName)
{
  for (plUInt32 i = 0; i < pEngine->GetGlobalFunctionCount(); ++i)
  {
    auto pFunc = pEngine->GetGlobalFunctionByIndex(i);

    if (plStringUtils::IsEqual(pFunc->GetNamespace(), szNamespace) && plStringUtils::IsEqual(pFunc->GetName(), szName))
    {
      return true;
    }
  }

  return false;
}

void plAngelScriptEngineSingleton::RegisterGenericFunction(const char* szTypeName, const plAbstractFunctionProperty* const pFunc, const plScriptableFunctionAttribute* pFuncAttr, bool bIsInherited)
{
  plStringBuilder sFuncName = pFunc->GetPropertyName();
  sFuncName.TrimWordStart("Reflection_");

  if (pFunc->GetReturnType() == plGetStaticRTTI<plVariant>())
  {
    plStringBuilder sFuncName2;

    sFuncName2.Set(sFuncName, "_asBool");
    RegisterSingleGenericFunction(sFuncName2, szTypeName, pFunc, pFuncAttr, bIsInherited, plGetStaticRTTI<bool>());

    sFuncName2.Set(sFuncName, "_asInt32");
    RegisterSingleGenericFunction(sFuncName2, szTypeName, pFunc, pFuncAttr, bIsInherited, plGetStaticRTTI<plInt32>());

    sFuncName2.Set(sFuncName, "_asFloat");
    RegisterSingleGenericFunction(sFuncName2, szTypeName, pFunc, pFuncAttr, bIsInherited, plGetStaticRTTI<float>());

    sFuncName2.Set(sFuncName, "_asTime");
    RegisterSingleGenericFunction(sFuncName2, szTypeName, pFunc, pFuncAttr, bIsInherited, plGetStaticRTTI<plTime>());

    sFuncName2.Set(sFuncName, "_asAngle");
    RegisterSingleGenericFunction(sFuncName2, szTypeName, pFunc, pFuncAttr, bIsInherited, plGetStaticRTTI<plAngle>());

    sFuncName2.Set(sFuncName, "_asVec2");
    RegisterSingleGenericFunction(sFuncName2, szTypeName, pFunc, pFuncAttr, bIsInherited, plGetStaticRTTI<plVec2>());

    sFuncName2.Set(sFuncName, "_asVec3");
    RegisterSingleGenericFunction(sFuncName2, szTypeName, pFunc, pFuncAttr, bIsInherited, plGetStaticRTTI<plVec3>());

    sFuncName2.Set(sFuncName, "_asVec4");
    RegisterSingleGenericFunction(sFuncName2, szTypeName, pFunc, pFuncAttr, bIsInherited, plGetStaticRTTI<plVec4>());

    sFuncName2.Set(sFuncName, "_asQuat");
    RegisterSingleGenericFunction(sFuncName2, szTypeName, pFunc, pFuncAttr, bIsInherited, plGetStaticRTTI<plQuat>());

    sFuncName2.Set(sFuncName, "_asColor");
    RegisterSingleGenericFunction(sFuncName2, szTypeName, pFunc, pFuncAttr, bIsInherited, plGetStaticRTTI<plColor>());

    sFuncName2.Set(sFuncName, "_asString");
    RegisterSingleGenericFunction(sFuncName2, szTypeName, pFunc, pFuncAttr, bIsInherited, plGetStaticRTTI<plString>());

    sFuncName2.Set(sFuncName, "_asGameObjectHandle");
    RegisterSingleGenericFunction(sFuncName2, szTypeName, pFunc, pFuncAttr, bIsInherited, plGetStaticRTTI<plGameObjectHandle>());

    sFuncName2.Set(sFuncName, "_asComponentHandle");
    RegisterSingleGenericFunction(sFuncName2, szTypeName, pFunc, pFuncAttr, bIsInherited, plGetStaticRTTI<plComponentHandle>());
  }
  else
  {
    RegisterSingleGenericFunction(sFuncName, szTypeName, pFunc, pFuncAttr, bIsInherited, pFunc->GetReturnType());
  }
}

void plAngelScriptEngineSingleton::RegisterSingleGenericFunction(const char* szFuncName, const char* szTypeName, const plAbstractFunctionProperty* const pFunc, const plScriptableFunctionAttribute* pFuncAttr, bool bIsInherited, const plRTTI* pReturnType)
{
  bool bVarArgs = false;

  if (const plDynamicPinAttribute* pVarArgsAttr = pFunc->GetAttributeByType<plDynamicPinAttribute>())
  {
    PL_ASSERT_DEV(pVarArgsAttr->GetProperty() == pFuncAttr->GetArgumentName(pFuncAttr->GetArgumentCount() - 1), "Var args must be on the last argument of the function");
    bVarArgs = true;
  }

  plStringBuilder decl;

  if (!AppendType(decl, pReturnType, nullptr, plInvalidIndex))
  {
    return;
  }

  plStringBuilder sNamespace;
  decl.Append(" ");

  if (pFunc->GetFunctionType() == plFunctionType::StaticMember)
  {
    // turn things like 'plScriptExtensionClass_CVar' into 'plCVar'
    if (const char* szUnderScore = plStringUtils::FindLastSubString(szTypeName, "_"))
    {
      if (plStringUtils::StartsWith(szTypeName, "pl"))
      {
        sNamespace.Append("pl");
      }

      sNamespace.Append(szUnderScore + 1);
    }
    else
    {
      sNamespace.Append(szTypeName);
    }
  }

  decl.Append(szFuncName, "(");

  plHybridArray<const plFunctionArgumentAttributes*, 8> argAttributes;
  if (const plFunctionArgumentAttributes* pArgAttr = pFunc->GetAttributeByType<plFunctionArgumentAttributes>())
  {
    argAttributes.SetCount(pFunc->GetArgumentCount());
    CollectFunctionArgumentAttributes(pFunc, argAttributes);
  }

  bool bHasDefaultArgs = false;
  plVariant defaultValue;

  const plUInt32 uiArgCount = bVarArgs ? pFunc->GetArgumentCount() - 1 : pFunc->GetArgumentCount();
  for (plUInt32 uiArg = 0; uiArg < uiArgCount; ++uiArg)
  {
    if (!AppendFuncArgs(decl, pFunc, pFuncAttr, uiArg))
      return;

    if (bVarArgs)
    {
      // start with 0 arguments
      decl.TrimRight(" ,");
    }
    else
    {
      if (decl.EndsWith("(") || decl.EndsWith(", "))
      {
        decl.TrimRight(" ,");
        continue;
      }

      const plRTTI* pArgType = pFunc->GetArgumentType(uiArg);

      if (const char* szName = pFuncAttr->GetArgumentName(uiArg))
      {
        decl.Append(" ", szName);
      }

      if (bHasDefaultArgs)
      {
        defaultValue = plReflectionUtils::GetDefaultVariantFromType(pArgType);
      }

      if (!argAttributes.IsEmpty() && argAttributes[uiArg])
      {
        for (auto pArgAttr : argAttributes[uiArg]->GetArgumentAttributes())
        {
          if (const plDefaultValueAttribute* pDef = plDynamicCast<const plDefaultValueAttribute*>(pArgAttr))
          {
            bHasDefaultArgs = true;
            defaultValue = pDef->GetValue();
          }
        }
      }

      if (bHasDefaultArgs)
      {
        const bool bIsEnum = pArgType->GetTypeFlags().IsAnySet(plTypeFlags::IsEnum | plTypeFlags::Bitflags);

        decl.Append(" = ");

        if (bIsEnum)
        {
          decl.Append(pArgType->GetTypeName(), "(");
        }

        if (defaultValue.IsValid())
        {
          plVariantType::Enum expectedType = bIsEnum ? plVariantType::Int64 : pArgType->GetVariantType();

          decl.Append(plAngelScriptUtils::DefaultValueToString(defaultValue, expectedType));
        }
        else
        {
          decl.Append("0"); // fallback for enums
        }

        if (bIsEnum)
        {
          decl.Append(")");
        }
      }
    }
  }

  intptr_t flags = 0;
  if (bIsInherited)
  {
    flags |= 0x01;
  }

  for (plUInt32 uiVarArgOpt = 0; uiVarArgOpt < 9; ++uiVarArgOpt)
  {
    decl.Append(")");

    if (pFunc->GetFunctionType() == plFunctionType::Member)
    {
      if (pFunc->GetFlags().IsSet(plPropertyFlags::Const))
        decl.Append(" const");

      // only register methods that have not been registered before
      // this allows us to register more optimized versions first
      if (m_pEngine->GetTypeInfoByName(szTypeName)->GetMethodByDecl(decl) == nullptr)
      {
        const int funcID = m_pEngine->RegisterObjectMethod(szTypeName, decl, asFUNCTION(plAngelScriptUtils::MakeGenericFunctionCall), asCALL_GENERIC, (void*)pFunc);
        AS_CHECK(funcID);

        m_pEngine->GetFunctionById(funcID)->SetUserData(reinterpret_cast<void*>(flags), plAsUserData::FuncFlags);
      }
    }
    else if (pFunc->GetFunctionType() == plFunctionType::StaticMember)
    {
      m_pEngine->SetDefaultNamespace(sNamespace);

      // only register functions that have not been registered before
      // this allows us to register more optimized versions first
      if (uiVarArgOpt > 0 || !ExistsGlobalFunc(m_pEngine, sNamespace, szFuncName))
      {
        const int funcID = m_pEngine->RegisterGlobalFunction(decl, asFUNCTION(plAngelScriptUtils::MakeGenericFunctionCall), asCALL_GENERIC, (void*)pFunc);
        AS_CHECK(funcID);

        m_pEngine->GetFunctionById(funcID)->SetUserData(reinterpret_cast<void*>(flags), plAsUserData::FuncFlags);
      }

      m_pEngine->SetDefaultNamespace("");
    }

    if (!bVarArgs)
      break;

    decl.Shrink(0, 1);
    decl.AppendFormat(", ?& in VarArg{}", uiVarArgOpt + 1);
  }
}
