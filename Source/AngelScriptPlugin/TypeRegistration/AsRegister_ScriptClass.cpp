#include <AngelScriptPlugin/AngelScriptPluginPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Runtime/AsEngineSingleton.h>
#include <AngelScriptPlugin/Runtime/AsInstance.h>
#include <Core/Scripting/ScriptComponent.h>

static plGameObject* GetAngelScriptOwnerObject(asIScriptObject* pSelf)
{
  if (pSelf)
  {
    plAngelScriptInstance* pInstance = (plAngelScriptInstance*)pSelf->GetUserData(plAsUserData::ScriptInstancePtr);
    pSelf->Release();

    return pInstance->GetOwnerComponent()->GetOwner();
  }

  return nullptr;
}

static plWorld* GetAngelScriptOwnerWorld(asIScriptObject* pSelf)
{
  if (pSelf)
  {
    plAngelScriptInstance* pInstance = (plAngelScriptInstance*)pSelf->GetUserData(plAsUserData::ScriptInstancePtr);
    pSelf->Release();

    return pInstance->GetOwnerComponent()->GetWorld();
  }

  return nullptr;
}

static plScriptComponent* GetAngelScriptOwnerComponent(asIScriptObject* pSelf)
{
  if (pSelf)
  {
    plAngelScriptInstance* pInstance = (plAngelScriptInstance*)pSelf->GetUserData(plAsUserData::ScriptInstancePtr);
    pSelf->Release();

    return pInstance->GetOwnerComponent();
  }

  return nullptr;
}


void plAngelScriptEngineSingleton::Register_plAngelScriptClass()
{
  AS_CHECK(m_pEngine->RegisterInterface("plIAngelScriptClass"));

  AS_CHECK(m_pEngine->RegisterGlobalFunction("plGameObject@ GetScriptOwnerObject(plIAngelScriptClass@ self)", asFUNCTION(GetAngelScriptOwnerObject), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("plScriptComponent@ GetScriptOwnerComponent(plIAngelScriptClass@ self)", asFUNCTION(GetAngelScriptOwnerComponent), asCALL_CDECL));
  AS_CHECK(m_pEngine->RegisterGlobalFunction("plWorld@ GetScriptOwnerWorld(plIAngelScriptClass@ self)", asFUNCTION(GetAngelScriptOwnerWorld), asCALL_CDECL));

  const char* szClassCode = R"(
shared class plAngelScriptClass : plIAngelScriptClass
{
    plScriptComponent@ GetOwnerComponent()
    {
        return GetScriptOwnerComponent(@this);
    }

    plGameObject@ GetOwner()
    {
        return GetScriptOwnerObject(@this);
    }

    plWorld@ GetWorld()
    {
        return GetScriptOwnerWorld(@this);
    }

    void SetUpdateInterval(plTime interval)
    {
      GetScriptOwnerComponent(@this).UpdateInterval = interval;
    }
}
    )";

  if (SetModuleCode("Builtin_AngelScriptClass", szClassCode, false) == nullptr)
  {
    PL_REPORT_FAILURE("Failed to register plAngelScriptClass class");
  }

  AS_CHECK(m_pEngine->RegisterObjectMethod("plScriptComponent", "void BroadcastEventMsg(const plEventMessage& in msg)", asMETHOD(plScriptComponent, BroadcastEventMsg), asCALL_THISCALL));
}
