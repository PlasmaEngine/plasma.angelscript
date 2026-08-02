#include <AngelScriptPlugin/AngelScriptPluginPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Runtime/AsFunctionDispatch.h>
#include <AngelScriptPlugin/Runtime/AsInstance.h>
#include <AngelScriptPlugin/Utils/AngelScriptUtils.h>
#include <Core/Scripting/ScriptComponent.h>
#include <Core/Scripting/ScriptWorldModule.h>

// clang-format off
PL_IMPLEMENT_MESSAGE_TYPE(plMsgDeliverAngelScriptMsg);
PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plMsgDeliverAngelScriptMsg, 1, plRTTIDefaultAllocator<plMsgDeliverAngelScriptMsg>)
PL_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

plAngelScriptFunctionProperty::plAngelScriptFunctionProperty(plStringView sName, asIScriptFunction* pFunction)
  : plScriptFunctionProperty(sName)
{
  m_pAsFunction = pFunction;
  m_pAsFunction->AddRef();
}

plAngelScriptFunctionProperty::~plAngelScriptFunctionProperty()
{
  if (m_pAsFunction)
  {
    m_pAsFunction->Release();
    m_pAsFunction = nullptr;
  }
}

void plAngelScriptFunctionProperty::Execute(void* pInstance, plArrayPtr<plVariant> arguments, plVariant& out_returnValue) const
{
  if (m_pAsFunction)
  {
    auto pScriptInstance = static_cast<plAngelScriptInstance*>(pInstance);
    auto pContext = pScriptInstance->GetContext();

    bool bPush = false;
    if (pContext->GetState() == asEContextState::asEXECUTION_ACTIVE)
    {
      bPush = true;
      AS_CHECK(pContext->PushState());
    }

    plAngelScriptUtils::SetThreadLocalWorld(pScriptInstance->GetWorld());

    plTime tDiff;

    if (pContext->Prepare(m_pAsFunction) >= 0)
    {
      PL_ASSERT_DEBUG(pScriptInstance->GetObject(), "Invalid script object");
      pContext->SetObject(pScriptInstance->GetObject());

      if (m_pAsFunction->GetParamCount() > 0)
      {
        tDiff = arguments[0].Get<plTime>();
        AS_CHECK(pContext->SetArgObject(0, &tDiff));
      }

      AS_CHECK(pContext->Execute());
    }

    if (bPush)
    {
      AS_CHECK(pContext->PopState());
    }
  }
}

//////////////////////////////////////////////////////////////////////////

plAngelScriptMessageHandler::plAngelScriptMessageHandler(const plScriptMessageDesc& desc, asIScriptFunction* pFunction)
  : plScriptMessageHandler(desc)
{
  m_DispatchFunc = &Dispatch;

  m_pAsFunction = pFunction;
  m_pAsFunction->AddRef();
}

plAngelScriptMessageHandler::~plAngelScriptMessageHandler()
{
  if (m_pAsFunction)
  {
    m_pAsFunction->Release();
    m_pAsFunction = nullptr;
  }
}

void plAngelScriptMessageHandler::Dispatch(plAbstractMessageHandler* pSelf, void* pInstance, plMessage& ref_msg)
{
  auto pScriptComp = static_cast<plScriptComponent*>(pInstance);

  auto pThis = static_cast<plAngelScriptMessageHandler*>(pSelf);
  auto pScriptInstance = static_cast<plAngelScriptInstance*>(pScriptComp->GetScriptInstance());
  auto pContext = pScriptInstance->GetContext();

  bool bPush = false;
  if (pContext->GetState() == asEContextState::asEXECUTION_ACTIVE)
  {
    bPush = true;
    AS_CHECK(pContext->PushState());
  }

  plAngelScriptUtils::SetThreadLocalWorld(pScriptInstance->GetWorld());

  if (pContext->Prepare(pThis->m_pAsFunction) >= 0)
  {
    PL_ASSERT_DEBUG(pScriptInstance->GetObject(), "Invalid script object");
    AS_CHECK(pContext->SetObject(pScriptInstance->GetObject()));
    AS_CHECK(pContext->SetArgObject(0, &ref_msg));
    AS_CHECK(pContext->Execute());
  }

  if (bPush)
  {
    AS_CHECK(pContext->PopState());
  }
}

//////////////////////////////////////////////////////////////////////////

plAngelScriptCustomAsMessageHandler::plAngelScriptCustomAsMessageHandler(const plScriptMessageDesc& desc)
  : plScriptMessageHandler(desc)
{
  m_DispatchFunc = &Dispatch;
}

plAngelScriptCustomAsMessageHandler::~plAngelScriptCustomAsMessageHandler()
{
  for (auto& r : m_Receivers)
  {
    r.m_pAsFunction->Release();
    r.m_pAsFunction = nullptr;
  }
}


void plAngelScriptCustomAsMessageHandler::AddReceiver(asIScriptFunction* pFunction, const char* szArgType)
{
  auto& r = m_Receivers.ExpandAndGetRef();
  r.m_pAsFunction = pFunction;
  r.m_pAsFunction->AddRef();
  r.m_sArgType.Assign(szArgType);
}

void plAngelScriptCustomAsMessageHandler::Dispatch(plAbstractMessageHandler* pSelf, void* pInstance, plMessage& ref_msg)
{
  auto pThis = static_cast<plAngelScriptCustomAsMessageHandler*>(pSelf);
  plMsgDeliverAngelScriptMsg& asMsg = static_cast<plMsgDeliverAngelScriptMsg&>(ref_msg);

  auto pMsgObj = reinterpret_cast<asIScriptObject*>(asMsg.m_pAsMsg);
  const char* szObjType = pMsgObj->GetObjectType()->GetName();
  const plTempHashedString sObjType(szObjType);

  for (const auto& r : pThis->m_Receivers)
  {
    if (r.m_sArgType != sObjType)
      continue;

    auto pScriptComp = static_cast<plScriptComponent*>(pInstance);
    auto pScriptInstance = static_cast<plAngelScriptInstance*>(pScriptComp->GetScriptInstance());
    PL_ASSERT_DEBUG(pScriptInstance->GetObject(), "Invalid script object");

    auto pContext = pScriptInstance->GetContext();

    bool bPush = false;
    if (pContext->GetState() == asEContextState::asEXECUTION_ACTIVE)
    {
      bPush = true;
      AS_CHECK(pContext->PushState());
    }

    plAngelScriptUtils::SetThreadLocalWorld(pScriptInstance->GetWorld());

    if (pContext->Prepare(r.m_pAsFunction) >= 0)
    {

      AS_CHECK(pContext->SetObject(pScriptInstance->GetObject()));
      AS_CHECK(pContext->SetArgObject(0, pMsgObj));
      AS_CHECK(pContext->Execute());
    }

    if (bPush)
    {
      AS_CHECK(pContext->PopState());
    }

    break;
  }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

plMsgDeliverAngelScriptMsg::~plMsgDeliverAngelScriptMsg()
{
  if (m_bRelease)
  {
    auto pObj = reinterpret_cast<asIScriptObject*>(m_pAsMsg);
    pObj->Release();
  }
}

plMsgDeliverAngelScriptMsg::plMsgDeliverAngelScriptMsg(const plMsgDeliverAngelScriptMsg& rhs)
{
  *this = rhs;
}

plMsgDeliverAngelScriptMsg::plMsgDeliverAngelScriptMsg(plMsgDeliverAngelScriptMsg&& rhs)
{
  *this = std::move(rhs);
}

void plMsgDeliverAngelScriptMsg::operator=(const plMsgDeliverAngelScriptMsg& rhs)
{
  plMemoryUtils::RawByteCopy(this, &rhs, sizeof(plMsgDeliverAngelScriptMsg));

  if (m_bRelease)
  {
    auto pObj = reinterpret_cast<asIScriptObject*>(m_pAsMsg);
    pObj->AddRef();
  }
}

void plMsgDeliverAngelScriptMsg::operator=(plMsgDeliverAngelScriptMsg&& rhs)
{
  m_bRelease = rhs.m_bRelease;
  m_pAsMsg = rhs.m_pAsMsg;
  rhs.m_bRelease = false;
  rhs.m_pAsMsg = nullptr;
}
