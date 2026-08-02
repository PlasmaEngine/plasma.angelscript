#include <AngelScriptPlugin/AngelScriptPluginPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Runtime/AsEngineSingleton.h>
#include <AngelScriptPlugin/Runtime/AsInstance.h>
#include <AngelScriptPlugin/Utils/AngelScriptUtils.h>
#include <Core/Scripting/ScriptComponent.h>

plAngelScriptInstance::plAngelScriptInstance(plReflectedClass& inout_owner, plWorld* pWorld, asIScriptModule* pModule, const char* szObjectTypeName)
  : plScriptInstance(inout_owner, pWorld)
{
  PL_ASSERT_DEBUG(inout_owner.GetDynamicRTTI()->IsDerivedFrom<plComponent>(), "Invalid owner");

  m_pOwnerComponent = static_cast<plScriptComponent*>(&inout_owner);

  auto pAsEngine = plAngelScriptEngineSingleton::GetSingleton();

  m_pContext = pAsEngine->GetEngine()->CreateContext();
  AS_CHECK(m_pContext->SetExceptionCallback(asMETHOD(plAngelScriptInstance, ExceptionCallback), this, asCALL_THISCALL));

  if (asITypeInfo* pClassType = pModule->GetTypeInfoByName(szObjectTypeName))
  {
    if (auto pFactory = pClassType->GetFactoryByIndex(0))
    {
      AS_CHECK(m_pContext->Prepare(pFactory));
      AS_CHECK(m_pContext->Execute());

      m_pObject = (asIScriptObject*)m_pContext->GetReturnObject();

      if (m_pObject)
      {
        m_pObject->AddRef();
        m_pObject->SetUserData(this, plAsUserData::ScriptInstancePtr);
        return;
      }
    }
  }

  plLog::Error("Failed to create AngelScript object of type '{}'", szObjectTypeName);
}

plAngelScriptInstance::~plAngelScriptInstance()
{
  if (m_pObject)
  {
    m_pObject->Release();
    m_pObject = nullptr;
  }

  if (m_pContext)
  {
    PL_ASSERT_DEBUG(!m_pContext->IsNested(), "Invalid time to release context!");

    m_pContext->Release();
    m_pContext = nullptr;
  }
}

void plAngelScriptInstance::SetInstanceVariable(const plHashedString& sName, const plVariant& value)
{
  if (!m_pObject)
    return;

  // TODO AngelScript: this could be a more efficient lookup instead of a search

  for (plUInt32 i = 0; i < m_pObject->GetPropertyCount(); ++i)
  {
    if (sName == m_pObject->GetPropertyName(i))
    {
      const int typeId = m_pObject->GetPropertyTypeId(i);
      void* pProp = m_pObject->GetAddressOfProperty(i);

      plAngelScriptUtils::WriteToAsTypeAtLocation(m_pObject->GetEngine(), typeId, pProp, value).AssertSuccess();
      return;
    }
  }

  plLog::Error("The variable '{}' doesn't exist in the Angel Script.", sName);
}

plVariant plAngelScriptInstance::GetInstanceVariable(const plHashedString& sName)
{
  PL_ASSERT_NOT_IMPLEMENTED;
  return {};
}

void plAngelScriptInstance::ExceptionCallback(asIScriptContext* pContext)
{
  PL_LOG_BLOCK("AS Exception", m_pOwnerComponent->GetScriptClass().GetResourceIdOrDescription());

  plLog::Error("AS Exception '{}' - in '{}'", pContext->GetExceptionString(), m_pOwnerComponent->GetScriptClass().GetResourceIdOrDescription());

  const plUInt32 uiNumLevels = pContext->GetCallstackSize();

  for (plUInt32 i = 0; i < uiNumLevels; ++i)
  {
    if (asIScriptFunction* pFunc = pContext->GetFunction(i))
    {
      plStringBuilder line("  ");

      if (!plStringUtils::IsNullOrEmpty(pFunc->GetNamespace()))
      {
        line.Append(pFunc->GetNamespace(), "::");
      }

      if (!plStringUtils::IsNullOrEmpty(pFunc->GetObjectName()))
      {
        line.Append(pFunc->GetObjectName(), "::");
      }

      const char* szSection = nullptr;
      int lineNbr = pContext->GetLineNumber(i, nullptr, &szSection);

      line.AppendFormat("{}() - Line {} in '{}'", pFunc->GetName(), lineNbr, szSection);

      plLog::Error(line);
    }
    else
    {
      plLog::Error("  <nested call>");
    }
  }
}
