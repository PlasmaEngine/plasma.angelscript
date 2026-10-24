#include <AngelScriptPlugin/AngelScriptPluginPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScript/source/add_on/scriptarray/scriptarray.h>
#include <AngelScript/source/add_on/scriptdictionary/scriptdictionary.h>
#include <AngelScriptPlugin/Runtime/AsDebugBackend.h>
#include <AngelScriptPlugin/Runtime/AsEngineSingleton.h>
#include <AngelScriptPlugin/Runtime/AsInstance.h>
#include <AngelScriptPlugin/Runtime/AsStringFactory.h>
#include <AngelScriptPlugin/Utils/AngelScriptUtils.h>
#include <Core/Scripting/ScriptComponent.h>
#include <Core/World/Component.h>
#include <Foundation/CodeUtils/Preprocessor.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Threading/Thread.h>
#include <Foundation/Types/Variant.h>

static plAsAllocatorType* g_pAsAllocator = nullptr;
static void OnThreadEvent(const plThreadEvent& e);

PL_IMPLEMENT_SINGLETON(plAngelScriptEngineSingleton);

// clang-format off
PL_BEGIN_SUBSYSTEM_DECLARATION(AngelScriptPlugin, AngelScriptEngineSingleton)

BEGIN_SUBSYSTEM_DEPENDENCIES
  "Foundation"
END_SUBSYSTEM_DEPENDENCIES

ON_CORESYSTEMS_SHUTDOWN
{
  // Created on demand by a headless tool, which never runs the high-level shutdown.
  if (plAngelScriptEngineSingleton* pEngine = plAngelScriptEngineSingleton::GetSingleton())
    PL_DEFAULT_DELETE(pEngine);

  if (g_pAsAllocator)
  {
    plThread::s_ThreadEvents.RemoveEventHandler(OnThreadEvent);
    PL_DEFAULT_DELETE(g_pAsAllocator);
  }
}

ON_HIGHLEVELSYSTEMS_STARTUP
{
  plAngelScriptEngineSingleton::GetOrCreate();
}

ON_HIGHLEVELSYSTEMS_SHUTDOWN
{
  plAngelScriptEngineSingleton* pDummy = plAngelScriptEngineSingleton::GetSingleton();
  PL_DEFAULT_DELETE(pDummy);
}

PL_END_SUBSYSTEM_DECLARATION;
// clang-format on



static void* plAsMalloc(size_t uiSize)
{
  return g_pAsAllocator->Allocate(uiSize, 8);
}

static void plAsFree(void* pPtr)
{
  g_pAsAllocator->Deallocate(pPtr);
}

static void AsThrow(plStringView sMsg)
{
  if (asIScriptContext* ctx = asGetActiveContext())
  {
    plStringBuilder tmp;
    ctx->SetException(sMsg.GetData(tmp), false);
  }
}

static void OnThreadEvent(const plThreadEvent& e)
{
  if (e.m_Type == plThreadEvent::Type::ClearThreadLocals)
  {
    asThreadCleanup();
  }
}

plAngelScriptEngineSingleton* plAngelScriptEngineSingleton::GetOrCreate()
{
  if (plAngelScriptEngineSingleton* pEngine = GetSingleton())
    return pEngine;

  if (g_pAsAllocator == nullptr)
  {
    g_pAsAllocator = PL_DEFAULT_NEW(plAsAllocatorType, "AngelScript", plFoundation::GetDefaultAllocator());
    plThread::s_ThreadEvents.AddEventHandler(OnThreadEvent);
  }

  return PL_DEFAULT_NEW(plAngelScriptEngineSingleton);
}

plAngelScriptEngineSingleton::plAngelScriptEngineSingleton()
  : m_SingletonRegistrar(this)
{
  PL_LOG_BLOCK("plAngelScriptEngineSingleton");

  asSetGlobalMemoryFunctions(plAsMalloc, plAsFree);

  m_pEngine = asCreateScriptEngine();
  // m_pEngine->SetEngineProperty(asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE, 1); // means we can't copy messages during PostMessage
  m_pEngine->SetEngineProperty(asEP_REQUIRE_ENUM_SCOPE, 1);
  m_pEngine->SetEngineProperty(asEP_DISALLOW_GLOBAL_VARS, 1);

  AS_CHECK(m_pEngine->SetMessageCallback(asMETHOD(plAngelScriptEngineSingleton, CompilerMessageCallback), this, asCALL_THISCALL));
  AS_CHECK(m_pEngine->SetTranslateAppExceptionCallback(asMETHOD(plAngelScriptEngineSingleton, ExceptionCallback), this, asCALL_THISCALL));

  m_pStringFactory = PL_DEFAULT_NEW(plAsStringFactory);

  AS_CHECK(m_pEngine->RegisterInterface("plAngelScriptMessage"));

  RegisterScriptArray(m_pEngine, true);
  // RegisterScriptDictionary(m_pEngine);

  RegisterStandardTypes();

  AS_CHECK(m_pEngine->RegisterGlobalFunction("void throw(plStringView)", asFUNCTION(AsThrow), asCALL_CDECL));

  m_pEngine->RegisterStringFactory("plStringView", m_pStringFactory);

  Register_ReflectedTypes();
  Register_GlobalReflectedFunctions();

  Register_plAngelScriptClass();

  AddForbiddenType("plStringView");
  AddForbiddenType("plStringBuilder");

  m_pDebugBackend = PL_DEFAULT_NEW(plAngelScriptDebugBackend);
  plScriptDebugger::RegisterBackend(m_pDebugBackend.Borrow());
}

plAngelScriptEngineSingleton::~plAngelScriptEngineSingleton()
{
  plScriptDebugger::UnregisterBackend(m_pDebugBackend.Borrow());
  m_pDebugBackend.Clear();

  m_pEngine->ShutDownAndRelease();

  if (m_pStringFactory)
  {
    plAsStringFactory* pFactor = (plAsStringFactory*)m_pStringFactory;
    PL_DEFAULT_DELETE(pFactor);
  }
}

void plAngelScriptEngineSingleton::AddForbiddenType(const char* szTypeName)
{
  asITypeInfo* pTypeInfo = m_pEngine->GetTypeInfoByName(szTypeName);
  PL_ASSERT_DEV(pTypeInfo != nullptr, "Type '{}' not found", szTypeName);

  m_ForbiddenTypes.PushBack(pTypeInfo);
}

bool plAngelScriptEngineSingleton::IsTypeForbidden(const asITypeInfo* pType) const
{
  return m_ForbiddenTypes.Contains(pType);
}

void plAngelScriptEngineSingleton::ExceptionCallback(asIScriptContext* pContext)
{
  plLog::Error("AngelScript: App-Exception: {}", pContext->GetExceptionString());
}

void plAngelScriptEngineSingleton::RegisterStandardTypes()
{
  PL_LOG_BLOCK("AS::RegisterStandardTypes");

  AS_CHECK(m_pEngine->RegisterTypedef("plInt8", "int8"));
  AS_CHECK(m_pEngine->RegisterTypedef("plInt16", "int16"));
  AS_CHECK(m_pEngine->RegisterTypedef("plInt32", "int32"));
  AS_CHECK(m_pEngine->RegisterTypedef("plInt64", "int64"));
  AS_CHECK(m_pEngine->RegisterTypedef("plUInt8", "uint8"));
  AS_CHECK(m_pEngine->RegisterTypedef("plUInt16", "uint16"));
  AS_CHECK(m_pEngine->RegisterTypedef("plUInt32", "uint32"));
  AS_CHECK(m_pEngine->RegisterTypedef("plUInt64", "uint64"));

  AS_CHECK(m_pEngine->RegisterObjectType("plRTTI", 0, asOBJ_REF | asOBJ_NOCOUNT));

  // TODO AngelScript: plResult ?

  RegisterPodValueType<plVec2>(asOBJ_APP_CLASS_ALLFLOATS | asOBJ_APP_CLASS_MORE_CONSTRUCTORS);
  RegisterPodValueType<plVec3>(asOBJ_APP_CLASS_ALLFLOATS | asOBJ_APP_CLASS_MORE_CONSTRUCTORS);
  RegisterPodValueType<plVec4>(asOBJ_APP_CLASS_ALLFLOATS | asOBJ_APP_CLASS_MORE_CONSTRUCTORS);
  RegisterPodValueType<plAngle>(asOBJ_APP_CLASS_ALLFLOATS | asOBJ_APP_CLASS_MORE_CONSTRUCTORS);
  RegisterPodValueType<plQuat>(asOBJ_APP_CLASS_ALLFLOATS | asOBJ_APP_CLASS_MORE_CONSTRUCTORS);
  RegisterPodValueType<plMat3>(asOBJ_APP_CLASS_ALLFLOATS);
  RegisterPodValueType<plMat4>(asOBJ_APP_CLASS_ALLFLOATS);
  RegisterPodValueType<plTransform>(asOBJ_APP_CLASS_ALLFLOATS | asOBJ_APP_CLASS_MORE_CONSTRUCTORS);
  RegisterPodValueType<plTime>(asOBJ_APP_CLASS_ALLFLOATS | asOBJ_APP_CLASS_ALIGN8);
  RegisterPodValueType<plColor>(asOBJ_APP_CLASS_ALLFLOATS | asOBJ_APP_CLASS_MORE_CONSTRUCTORS);
  RegisterPodValueType<plColorGammaUB>(asOBJ_APP_CLASS_ALLINTS | asOBJ_APP_CLASS_MORE_CONSTRUCTORS);
  RegisterPodValueType<plStringView>(asOBJ_APP_CLASS_ALLINTS | asOBJ_APP_CLASS_MORE_CONSTRUCTORS);
  RegisterPodValueType<plGameObjectHandle>(asOBJ_APP_CLASS_ALLINTS | asOBJ_APP_CLASS_MORE_CONSTRUCTORS);
  RegisterPodValueType<plComponentHandle>(asOBJ_APP_CLASS_ALLINTS | asOBJ_APP_CLASS_MORE_CONSTRUCTORS);
  RegisterPodValueType<plTempHashedString>(asOBJ_APP_CLASS_ALLINTS | asOBJ_APP_CLASS_MORE_CONSTRUCTORS);
  RegisterPodValueType<plHashedString>(asOBJ_APP_CLASS_ALLINTS | asOBJ_APP_CLASS_MORE_CONSTRUCTORS);

  RegisterNonPodValueType<plString>();
  RegisterNonPodValueType<plStringBuilder>();

  RegisterRefType<plGameObject>();
  RegisterRefType<plComponent>();
  RegisterRefType<plWorld>();
  RegisterRefType<plMessage>();
  RegisterRefType<plClock>();

  {
    AS_CHECK(m_pEngine->RegisterObjectType("plRandom", 0, asOBJ_REF | asOBJ_NOCOUNT));
    AddForbiddenType("plRandom");
  }

  Register_RTTI();
  Register_Vec2();
  Register_Vec3();
  Register_Vec4();
  Register_Angle();
  Register_Quat();
  Register_Transform();
  Register_GameObject();
  Register_Time();
  Register_Mat3();
  Register_Mat4();
  Register_World();
  Register_Clock();
  Register_StringView();
  Register_String();
  Register_StringBuilder();
  Register_HashedString();
  Register_TempHashedString();
  Register_Color();
  Register_ColorGammaUB();
  Register_Random();
  Register_Math();
  Register_Spatial();

  // TODO AngelScript: register these standard types
  // plBoundingBox
  // plBoundingSphere
  // plPlane
}



void plAngelScriptEngineSingleton::Register_ReflectedTypes()
{
  PL_LOG_BLOCK("Register_ReflectedTypes");

  if (plScriptBindingRegistry::CreateSnapshot(m_BindingSnapshot).Failed())
  {
    m_BindingSnapshot.Clear();
    plLog::Error("The shared script binding snapshot contains colliding stable IDs. Reflected AngelScript registration is disabled.");
    return;
  }

  Register_ReflectedType(plGetStaticRTTI<plComponent>(), false);
  Register_ReflectedType(plGetStaticRTTI<plMessage>(), true);

  Register_ExtraComponentFuncs();
}

void plAngelScriptEngineSingleton::Register_ExtraComponentFuncs()
{
  plDynamicArray<const plScriptBindingTypeDesc*> types;
  m_BindingSnapshot.GetTypesInReflectionOrder(types);
  for (const plScriptBindingTypeDesc* pType : types)
  {
    const plRTTI* pRtti = pType->m_pRtti;
    if (!pRtti->IsDerivedFrom<plComponent>())
      continue;

    if (pType->m_bHidden || pType->m_bExcludedFromScript)
      continue;

    intptr_t flags = 0;

    if (pRtti != plGetStaticRTTI<plComponent>())
    {
      // derived type
      flags = 0x01;
    }

    const char* compName = pType->m_sNativeName.GetData();

    int funcID;

    funcID = m_pEngine->RegisterObjectMethod(compName, "bool SendMessage(plMessage& inout ref_msg)", asMETHODPR(plComponent, SendMessage, (plMessage&), bool), asCALL_THISCALL);
    AS_CHECK(funcID);
    m_pEngine->GetFunctionById(funcID)->SetUserData(reinterpret_cast<void*>(flags), plAsUserData::FuncFlags);

    funcID = m_pEngine->RegisterObjectMethod(compName, "bool SendMessage(plMessage& inout ref_msg) const", asMETHODPR(plComponent, SendMessage, (plMessage&) const, bool), asCALL_THISCALL);
    AS_CHECK(funcID);
    m_pEngine->GetFunctionById(funcID)->SetUserData(reinterpret_cast<void*>(flags), plAsUserData::FuncFlags);

    funcID = m_pEngine->RegisterObjectMethod(compName, "void PostMessage(const plMessage& in msg, plTime delay = plTime::MakeZero(), plObjectMsgQueueType queueType = plObjectMsgQueueType::NextFrame) const", asMETHOD(plComponent, PostMessage), asCALL_THISCALL);
    AS_CHECK(funcID);
    m_pEngine->GetFunctionById(funcID)->SetUserData(reinterpret_cast<void*>(flags), plAsUserData::FuncFlags);

    funcID = m_pEngine->RegisterObjectMethod(compName, "plComponentHandle GetHandle() const", asMETHOD(plComponent, GetHandle), asCALL_THISCALL);
    AS_CHECK(funcID);
    m_pEngine->GetFunctionById(funcID)->SetUserData(reinterpret_cast<void*>(flags), plAsUserData::FuncFlags);
  }
}
