#include <AngelScriptPlugin/AngelScriptPluginPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Runtime/AsDebugBackend.h>
#include <AngelScriptPlugin/Runtime/AsEngineSingleton.h>
#include <Core/Scripting/ScriptDiagnostic.h>
#include <Core/World/GameObject.h>
#include <AngelScriptPlugin/Runtime/AsInstance.h>
#include <AngelScriptPlugin/Utils/AngelScriptUtils.h>
#include <Core/Scripting/ScriptComponent.h>

namespace
{
  /// The script object's property of that name, or of the field that was renamed from it (`/// @was`). -1 if none.
  plInt32 FindPropertyByName(asIScriptObject* pObject, plStringView sName)
  {
    for (plUInt32 i = 0; i < pObject->GetPropertyCount(); ++i)
    {
      if (sName == pObject->GetPropertyName(i))
        return static_cast<plInt32>(i);
    }
    return -1;
  }

  plInt32 FindProperty(asIScriptObject* pObject, plStringView sName)
  {
    const plInt32 iProperty = FindPropertyByName(pObject, sName);
    if (iProperty >= 0)
      return iProperty;

    const plStringView sRenamed = plAngelScriptEngineSingleton::GetSingleton()->FindRenamedField(pObject->GetObjectType()->GetModule(), sName);
    return sRenamed.IsEmpty() ? -1 : FindPropertyByName(pObject, sRenamed);
  }
} // namespace

plAngelScriptInstance::plAngelScriptInstance(plReflectedClass& inout_owner, plWorld* pWorld, asIScriptModule* pModule, const char* szObjectTypeName)
  : plScriptInstance(inout_owner, pWorld)
{
  PL_ASSERT_DEBUG(inout_owner.GetDynamicRTTI()->IsDerivedFrom<plComponent>(), "Invalid owner");

  m_pOwnerComponent = static_cast<plScriptComponent*>(&inout_owner);

  auto pAsEngine = plAngelScriptEngineSingleton::GetSingleton();

  m_pContext = pAsEngine->GetEngine()->CreateContext();
  AS_CHECK(m_pContext->SetExceptionCallback(asMETHOD(plAngelScriptInstance, ExceptionCallback), this, asCALL_THISCALL));
  pAsEngine->AddInstance(this);

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
  if (plAngelScriptEngineSingleton* pAsEngine = plAngelScriptEngineSingleton::GetSingleton())
    pAsEngine->RemoveInstance(this);

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

  const plInt32 iProperty = FindProperty(m_pObject, sName);
  if (iProperty >= 0)
  {
    const int typeId = m_pObject->GetPropertyTypeId(iProperty);
    void* pProp = m_pObject->GetAddressOfProperty(iProperty);

    plAngelScriptUtils::WriteToAsTypeAtLocation(m_pObject->GetEngine(), typeId, pProp, value).AssertSuccess();
    return;
  }

  plLog::Error("The variable '{}' doesn't exist in the Angel Script.", sName);
}

plVariant plAngelScriptInstance::GetInstanceVariable(const plHashedString& sName)
{
  if (!m_pObject)
    return {};

  for (plUInt32 i = 0; i < m_pObject->GetPropertyCount(); ++i)
  {
    if (sName == m_pObject->GetPropertyName(i))
    {
      plVariant value;
      if (ReadMember(i, value).Succeeded())
        return value;
      return {};
    }
  }

  return {};
}

plResult plAngelScriptInstance::ReadMember(plUInt32 uiIndex, plVariant& out_value) const
{
  const int typeId = m_pObject->GetPropertyTypeId(uiIndex);

  // Templates (array<T>) and script objects have no RTTI to convert through.
  if ((typeId & (asTYPEID_OBJHANDLE | asTYPEID_SCRIPTOBJECT | asTYPEID_TEMPLATE)) != 0)
    return PL_FAILURE;

  if (typeId > asTYPEID_DOUBLE)
  {
    asITypeInfo* pType = m_pObject->GetEngine()->GetTypeInfoById(typeId);
    if (pType == nullptr || ((pType->GetFlags() & asOBJ_ENUM) == 0 && pType->GetUserData(plAsUserData::RttiPtr) == nullptr))
      return PL_FAILURE;
  }

  return plAngelScriptUtils::ReadFromAsTypeAtLocation(m_pObject->GetEngine(), typeId, m_pObject->GetAddressOfProperty(uiIndex), out_value);
}

void plAngelScriptInstance::SetInstanceVariables(const plArrayMap<plHashedString, plVariant>& parameters)
{
  if (!m_pObject)
    return;

  for (plUInt32 p = 0; p < parameters.GetCount(); ++p)
  {
    const plInt32 i = FindProperty(m_pObject, parameters.GetKey(p));
    if (i >= 0)
      plAngelScriptUtils::WriteToAsTypeAtLocation(m_pObject->GetEngine(), m_pObject->GetPropertyTypeId(i), m_pObject->GetAddressOfProperty(i), parameters.GetValue(p)).IgnoreResult();
  }
}

void plAngelScriptInstance::GetInstanceVariables(plArrayMap<plHashedString, plVariant>& out_parameters) const
{
  out_parameters.Clear();

  if (!m_pObject)
    return;

  for (plUInt32 i = 0; i < m_pObject->GetPropertyCount(); ++i)
  {
    plVariant value;
    if (ReadMember(i, value).Succeeded() && value.IsValid())
    {
      plHashedString sName;
      sName.Assign(m_pObject->GetPropertyName(i));
      out_parameters.Insert(sName, value);
    }
  }
}

void plAngelScriptInstance::ExceptionCallback(asIScriptContext* pContext)
{
  PL_LOG_BLOCK("AS Exception", m_pOwnerComponent->GetScriptClass().GetResourceIdOrDescription());

  plLog::Error("AS Exception '{}' - in '{}'", pContext->GetExceptionString(), m_pOwnerComponent->GetScriptClass().GetResourceIdOrDescription());

  // Each level maps back to the original file and line; the raw row would name the flattened "Main" section.
  plStringBuilder sStack;
  const plUInt32 uiNumLevels = pContext->GetCallstackSize();

  for (plUInt32 i = 0; i < uiNumLevels; ++i)
  {
    if (asIScriptFunction* pFunc = pContext->GetFunction(i))
    {
      const plScriptDebugLocation location = plAngelScriptDebugBackend::GetLocation(pContext, i);

      plStringBuilder line("  ");
      line.Append(plAngelScriptUtils::GetNiceFunctionDeclaration(pFunc, true, true));
      line.AppendFormat(" - Line {} in '{}'", location.m_uiLine, location.m_sFile);

      plLog::Error(line);
      sStack.Append(line.GetView().GetSubString(2, line.GetCharacterCount()), "\n");
    }
    else
    {
      plLog::Error("  <nested call>");
      sStack.Append("<nested call>\n");
    }
  }

  const plScriptDebugLocation top = uiNumLevels > 0 ? plAngelScriptDebugBackend::GetLocation(pContext, 0) : plScriptDebugLocation();

  plScriptDiagnostic diag;
  diag.m_sLanguage = "AngelScript";
  diag.m_Severity = plScriptDiagnosticSeverity::Error;
  diag.m_Origin = plScriptDiagnosticOrigin::Runtime;
  diag.m_sMessage = pContext->GetExceptionString();
  diag.m_sAsset = m_pOwnerComponent->GetScriptClass().GetResourceID();
  diag.m_sFile = top.m_sFile;
  diag.m_uiLine = top.m_uiLine;
  diag.m_uiColumn = top.m_uiColumn;
  diag.m_sInstance = m_pOwnerComponent->GetOwner()->GetName();
  diag.m_sStack = sStack;
  plScriptDiagnostics::Report(diag);

  plScriptDebugger::OnException(nullptr, top, pContext->GetExceptionString());
}
