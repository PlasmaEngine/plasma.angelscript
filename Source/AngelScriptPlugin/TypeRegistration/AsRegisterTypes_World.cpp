#include <AngelScriptPlugin/AngelScriptPluginPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Runtime/AsEngineSingleton.h>
#include <AngelScriptPlugin/Runtime/AsFunctionDispatch.h>
#include <AngelScriptPlugin/Utils/AngelScriptUtils.h>
#include <Core/World/GameObject.h>
#include <Core/World/World.h>

//////////////////////////////////////////////////////////////////////////
// plGameObject
//////////////////////////////////////////////////////////////////////////

void plGameObject_TryGetComponentOfBaseType(asIScriptGeneric* pGen)
{
  plGameObject* pObj = (plGameObject*)pGen->GetObject();
  int typeId = pGen->GetArgTypeId(0);

  if (auto info = pGen->GetEngine()->GetTypeInfoById(typeId))
  {
    if (const plRTTI* pRtti = static_cast<const plRTTI*>(info->GetUserData(plAsUserData::RttiPtr)))
    {
      plComponent* pComponent;
      if (pObj->TryGetComponentOfBaseType(pRtti, pComponent))
      {
        plComponent** ref = (plComponent**)pGen->GetArgAddress(0);
        *ref = pComponent;

        pGen->SetReturnByte(1);
        return;
      }
    }
  }

  pGen->SetReturnByte(0);
}

void plGameObject_CreateComponent(asIScriptGeneric* pGen)
{
  plGameObject* pObj = (plGameObject*)pGen->GetObject();
  plWorld* pWorld = pObj->GetWorld();
  const int typeId = pGen->GetArgTypeId(0);

  if (auto info = pGen->GetEngine()->GetTypeInfoById(typeId))
  {
    if (const plRTTI* pRtti = static_cast<const plRTTI*>(info->GetUserData(plAsUserData::RttiPtr)))
    {
      if (auto pCompMan = pWorld->GetOrCreateManagerForComponentType(pRtti))
      {
        plComponentHandle hComp = pCompMan->CreateComponent(pObj);

        plComponent* pComponent;
        if (pWorld->TryGetComponent(hComp, pComponent))
        {
          plComponent** ref = (plComponent**)pGen->GetArgAddress(0);
          *ref = pComponent;

          pGen->SetReturnByte(1);
          return;
        }
      }
    }
  }

  pGen->SetReturnByte(0);
}

void plGameObjectHandle_Construct(void* pMemory)
{
  new (pMemory) plGameObjectHandle();
}

void plComponentHandle_Construct(void* pMemory)
{
  new (pMemory) plComponentHandle();
}

void plGameObject_SendAsMessage(asIScriptGeneric* pGen)
{
  plGameObject* pObj = (plGameObject*)pGen->GetObject();
  void* pAsMsg = pGen->GetArgObject(0);

  plMsgDeliverAngelScriptMsg msg;
  msg.m_pAsMsg = pAsMsg;
  if (pObj->SendMessage(msg))
  {
    pGen->SetReturnByte(1);
  }
  else
  {
    pGen->SetReturnByte(0);
  }
}

void plGameObject_SendAsMessageConst(asIScriptGeneric* pGen)
{
  const plGameObject* pObj = (const plGameObject*)pGen->GetObject();
  void* pAsMsg = pGen->GetArgObject(0);

  plMsgDeliverAngelScriptMsg msg;
  msg.m_pAsMsg = pAsMsg;
  if (pObj->SendMessage(msg))
  {
    pGen->SetReturnByte(1);
  }
  else
  {
    pGen->SetReturnByte(0);
  }
}

void plGameObject_SendAsMessageRecursive(asIScriptGeneric* pGen)
{
  plGameObject* pObj = (plGameObject*)pGen->GetObject();
  void* pAsMsg = pGen->GetArgObject(0);

  plMsgDeliverAngelScriptMsg msg;
  msg.m_pAsMsg = pAsMsg;
  if (pObj->SendMessageRecursive(msg))
  {
    pGen->SetReturnByte(1);
  }
  else
  {
    pGen->SetReturnByte(0);
  }
}

void plGameObject_SendAsMessageRecursiveConst(asIScriptGeneric* pGen)
{
  const plGameObject* pObj = (const plGameObject*)pGen->GetObject();
  void* pAsMsg = pGen->GetArgObject(0);

  plMsgDeliverAngelScriptMsg msg;
  msg.m_pAsMsg = pAsMsg;
  if (pObj->SendMessageRecursive(msg))
  {
    pGen->SetReturnByte(1);
  }
  else
  {
    pGen->SetReturnByte(0);
  }
}

void plGameObject_PostAsMessage(asIScriptGeneric* pGen)
{
  plGameObject* pObj = (plGameObject*)pGen->GetObject();
  void* pAsMsg = pGen->GetArgObject(0);

  plTime delay = *((const plTime*)pGen->GetArgObject(1));
  plInt32 delivery = (plInt32)pGen->GetArgDWord(2);

  void* pMsgCopy = pGen->GetEngine()->CreateScriptObjectCopy(pAsMsg, reinterpret_cast<asIScriptObject*>(pAsMsg)->GetObjectType());
  PL_ASSERT_DEV(pMsgCopy != nullptr, "Failed to create copy of message");

  plMsgDeliverAngelScriptMsg msg;
  msg.m_bRelease = true;
  msg.m_pAsMsg = pMsgCopy;
  pObj->PostMessage(msg, delay, static_cast<plObjectMsgQueueType::Enum>(delivery));
}

void plGameObject_PostAsMessageRecursive(asIScriptGeneric* pGen)
{
  plGameObject* pObj = (plGameObject*)pGen->GetObject();
  void* pAsMsg = pGen->GetArgObject(0);

  plTime delay = *((const plTime*)pGen->GetArgObject(1));
  plInt32 delivery = (plInt32)pGen->GetArgDWord(2);

  void* pMsgCopy = pGen->GetEngine()->CreateScriptObjectCopy(pAsMsg, reinterpret_cast<asIScriptObject*>(pAsMsg)->GetObjectType());
  PL_ASSERT_DEV(pMsgCopy != nullptr, "Failed to create copy of message");

  plMsgDeliverAngelScriptMsg msg;
  msg.m_bRelease = true;
  msg.m_pAsMsg = pMsgCopy;
  pObj->PostMessageRecursive(msg, delay, static_cast<plObjectMsgQueueType::Enum>(delivery));
}

void plAngelScriptEngineSingleton::Register_GameObject()
{
  plAngelScriptUtils::RegisterEnumType(m_pEngine, plGetStaticRTTI<plObjectMsgQueueType>());
  plAngelScriptUtils::RegisterEnumType(m_pEngine, plGetStaticRTTI<plTransformPreservation>());

  {
    AS_CHECK(m_pEngine->RegisterObjectBehaviour("plGameObjectHandle", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(plGameObjectHandle_Construct), asCALL_CDECL_OBJFIRST));

    AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObjectHandle", "void Invalidate()", asMETHOD(plGameObjectHandle, Invalidate), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObjectHandle", "bool IsInvalidated() const", asMETHOD(plGameObjectHandle, IsInvalidated), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObjectHandle", "bool opEquals(plGameObjectHandle) const", asMETHODPR(plGameObjectHandle, operator==, (plGameObjectHandle) const, bool), asCALL_THISCALL));
  }

  {
    AS_CHECK(m_pEngine->RegisterObjectBehaviour("plComponentHandle", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(plComponentHandle_Construct), asCALL_CDECL_OBJFIRST));

    AS_CHECK(m_pEngine->RegisterObjectMethod("plComponentHandle", "void Invalidate()", asMETHOD(plComponentHandle, Invalidate), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plComponentHandle", "bool IsInvalidated() const", asMETHOD(plComponentHandle, IsInvalidated), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plComponentHandle", "bool opEquals(plComponentHandle) const", asMETHODPR(plComponentHandle, operator==, (plComponentHandle) const, bool), asCALL_THISCALL));
  }

  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plGameObjectHandle GetHandle() const", asMETHOD(plGameObject, GetHandle), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void MakeDynamic()", asMETHOD(plGameObject, MakeDynamic), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void MakeStatic()", asMETHOD(plGameObject, MakeStatic), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "bool IsDynamic() const", asMETHOD(plGameObject, IsDynamic), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "bool IsStatic() const", asMETHOD(plGameObject, IsStatic), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetActiveFlag(bool bActive)", asMETHOD(plGameObject, SetActiveFlag), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "bool GetActiveFlag() const", asMETHOD(plGameObject, GetActiveFlag), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "bool IsActive() const", asMETHOD(plGameObject, IsActive), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetName(plStringView sName)", asMETHODPR(plGameObject, SetName, (plStringView), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetName(const plHashedString& in sName)", asMETHODPR(plGameObject, SetName, (const plHashedString&), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plStringView GetName() const", asMETHOD(plGameObject, GetName), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "bool HasName(const plTempHashedString& in sName) const", asMETHOD(plGameObject, HasName), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetGlobalKey(plStringView sKey)", asMETHODPR(plGameObject, SetGlobalKey, (plStringView), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetGlobalKey(const plHashedString& in sKey)", asMETHODPR(plGameObject, SetGlobalKey, (const plHashedString&), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plStringView GetGlobalKey() const", asMETHOD(plGameObject, GetGlobalKey), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetParent(const plGameObjectHandle& in hParent, plTransformPreservation preserve = plTransformPreservation::PreserveGlobal)", asMETHOD(plGameObject, SetParent), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plGameObject@ GetParent() const", asMETHODPR(plGameObject, GetParent, () const, const plGameObject*), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plGameObject@ GetParent()", asMETHODPR(plGameObject, GetParent, (), plGameObject*), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void AddChild(const plGameObjectHandle& in hChild, plTransformPreservation preserve = plTransformPreservation::PreserveGlobal)", asMETHOD(plGameObject, AddChild), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void DetachChild(const plGameObjectHandle& in hChild, plTransformPreservation preserve = plTransformPreservation::PreserveGlobal)", asMETHOD(plGameObject, DetachChild), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "uint32 GetChildCount()", asMETHOD(plGameObject, GetChildCount), asCALL_THISCALL));
  // AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void GetChildren()", asMETHOD(plGameObject, GetChildren), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plGameObject@ FindChildByName(const plTempHashedString& in sName, bool bRecursive = true)", asMETHOD(plGameObject, FindChildByName), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plGameObject@ FindChildByPath(plStringView sPath)", asMETHOD(plGameObject, FindChildByPath), asCALL_THISCALL));
  // AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plGameObject@ SearchForChildByNameSequence(string)", asMETHOD(plGameObject, SearchForChildByNameSequence), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plWorld@ GetWorld()", asMETHODPR(plGameObject, GetWorld, (), plWorld*), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "const plWorld@ GetWorld() const", asMETHODPR(plGameObject, GetWorld, () const, const plWorld*), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetLocalPosition(const plVec3& in)", asMETHODPR(plGameObject, SetLocalPosition, (plVec3), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plVec3 GetLocalPosition() const", asMETHOD(plGameObject, GetLocalPosition), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetLocalRotation(const plQuat& in)", asMETHODPR(plGameObject, SetLocalRotation, (plQuat), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plQuat GetLocalRotation() const", asMETHOD(plGameObject, GetLocalRotation), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetLocalScaling(const plVec3& in)", asMETHODPR(plGameObject, SetLocalScaling, (plVec3), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plVec3 GetLocalScaling() const", asMETHOD(plGameObject, GetLocalScaling), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetLocalUniformScaling(float fScale)", asMETHODPR(plGameObject, SetLocalUniformScaling, (float), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "float GetLocalUniformScaling() const", asMETHOD(plGameObject, GetLocalUniformScaling), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plTransform GetLocalTransform() const", asMETHOD(plGameObject, GetLocalTransform), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetGlobalPosition(const plVec3& in)", asMETHODPR(plGameObject, SetGlobalPosition, (const plVec3&), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plVec3 GetGlobalPosition() const", asMETHOD(plGameObject, GetGlobalPosition), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetGlobalRotation(const plQuat& in)", asMETHODPR(plGameObject, SetGlobalRotation, (const plQuat&), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plQuat GetGlobalRotation() const", asMETHODPR(plGameObject, GetGlobalRotation, () const, plQuat), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetGlobalScaling(const plVec3& in)", asMETHODPR(plGameObject, SetGlobalScaling, (const plVec3&), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plVec3 GetGlobalScaling() const", asMETHOD(plGameObject, GetGlobalScaling), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetGlobalTransform(const plTransform& in)", asMETHODPR(plGameObject, SetGlobalTransform, (const plTransform&), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plTransform GetGlobalTransform() const", asMETHOD(plGameObject, GetGlobalTransform), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plTransform GetLastGlobalTransform() const", asMETHOD(plGameObject, GetLastGlobalTransform), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plVec3 GetGlobalDirForwards() const", asMETHOD(plGameObject, GetGlobalDirForwards), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plVec3 GetGlobalDirRight() const", asMETHOD(plGameObject, GetGlobalDirRight), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plVec3 GetGlobalDirUp() const", asMETHOD(plGameObject, GetGlobalDirUp), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetGlobalRotationToLookAt(const plVec3& in vTargetPosition, const plVec3& in vUp = plVec3(0, 0, 1))", asMETHOD(plGameObject, SetGlobalRotationToLookAt), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetGlobalTransformToLookAt(const plVec3& in vOwnPosition, const plVec3& in vTargetPosition, const plVec3& in vUp = plVec3(0, 0, 1))", asMETHOD(plGameObject, SetGlobalTransformToLookAt), asCALL_THISCALL));

#if PL_ENABLED(PL_GAMEOBJECT_VELOCITY)
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plVec3 GetLinearVelocity() const", asMETHOD(plGameObject, GetLinearVelocity), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plVec3 GetAngularVelocity() const", asMETHOD(plGameObject, GetAngularVelocity), asCALL_THISCALL));
#endif

  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void UpdateGlobalTransform()", asMETHOD(plGameObject, UpdateGlobalTransform), asCALL_THISCALL));

  // AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plBoundingBoxSphere GetLocalBounds() const", asMETHOD(plGameObject, GetLocalBounds), asCALL_THISCALL));
  // AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "plBoundingBoxSphere GetGlobalBounds() const", asMETHOD(plGameObject, GetGlobalBounds), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void UpdateLocalBounds()", asMETHOD(plGameObject, UpdateLocalBounds), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void UpdateGlobalBounds()", asMETHOD(plGameObject, UpdateGlobalBounds), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void UpdateGlobalTransformAndBounds()", asMETHOD(plGameObject, UpdateGlobalTransformAndBounds), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "bool TryGetComponentOfBaseType(const plRTTI@ pType, plComponent@& out pComponent)", asMETHODPR(plGameObject, TryGetComponentOfBaseType, (const plRTTI*, plComponent*&), bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "bool TryGetComponentOfBaseType(?& out pTypedComponent)", asFUNCTION(plGameObject_TryGetComponentOfBaseType), asCALL_GENERIC));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "bool CreateComponent(?& out pTypedComponent)", asFUNCTION(plGameObject_CreateComponent), asCALL_GENERIC));

  // AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void GetComponents()", asMETHOD(plGameObject, GetComponents), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "uint16 GetComponentVersion()", asMETHOD(plGameObject, GetComponentVersion), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "bool SendMessage(plMessage& inout)", asMETHODPR(plGameObject, SendMessage, (plMessage&), bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "bool SendMessage(plMessage& inout) const", asMETHODPR(plGameObject, SendMessage, (plMessage&) const, bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "bool SendMessageRecursive(plMessage& inout)", asMETHODPR(plGameObject, SendMessageRecursive, (plMessage&), bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "bool SendMessageRecursive(plMessage& inout) const", asMETHODPR(plGameObject, SendMessageRecursive, (plMessage&) const, bool), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void PostMessage(const plMessage& in, plTime delay, plObjectMsgQueueType delivery = plObjectMsgQueueType::NextFrame) const", asMETHOD(plGameObject, PostMessage), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void PostMessageRecursive(const plMessage& in, plTime delay, plObjectMsgQueueType delivery = plObjectMsgQueueType::NextFrame) const", asMETHOD(plGameObject, PostMessageRecursive), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "bool SendEventMessage(plMessage& inout, const plComponent@ pSender)", asMETHODPR(plGameObject, SendEventMessage, (plMessage&, const plComponent*), bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "bool SendEventMessage(plMessage& inout, const plComponent@ pSender) const", asMETHODPR(plGameObject, SendEventMessage, (plMessage&, const plComponent*) const, bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void PostEventMessage(const plMessage& in msg, const plComponent@ pSender, plTime delay, plObjectMsgQueueType delivery = plObjectMsgQueueType::NextFrame) const", asMETHOD(plGameObject, PostEventMessage), asCALL_THISCALL));

  // GetTags
  // SetTags
  // SetTag
  // RemoveTag
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "bool HasTag(const plTempHashedString& in sTagName) const", asMETHOD(plGameObject, HasTag), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "uint16 GetTeamID()", asMETHOD(plGameObject, GetTeamID), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetTeamID(uint16 id)", asMETHOD(plGameObject, SetTeamID), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "uint32 GetStableRandomSeed()", asMETHOD(plGameObject, GetStableRandomSeed), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SetStableRandomSeed(uint32 seed)", asMETHOD(plGameObject, SetStableRandomSeed), asCALL_THISCALL));

  // GetVisibilityState

  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SendMessage(plAngelScriptMessage& inout)", asFUNCTION(plGameObject_SendAsMessage), asCALL_GENERIC));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SendMessage(plAngelScriptMessage& inout) const", asFUNCTION(plGameObject_SendAsMessageConst), asCALL_GENERIC));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SendMessageRecursive(plAngelScriptMessage& inout)", asFUNCTION(plGameObject_SendAsMessageRecursive), asCALL_GENERIC));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void SendMessageRecursive(plAngelScriptMessage& inout) const", asFUNCTION(plGameObject_SendAsMessageRecursiveConst), asCALL_GENERIC));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void PostMessage(const plAngelScriptMessage& in, plTime delay, plObjectMsgQueueType delivery = plObjectMsgQueueType::NextFrame)", asFUNCTION(plGameObject_PostAsMessage), asCALL_GENERIC));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plGameObject", "void PostMessageRecursive(const plAngelScriptMessage& in, plTime delay, plObjectMsgQueueType delivery = plObjectMsgQueueType::NextFrame)", asFUNCTION(plGameObject_PostAsMessageRecursive), asCALL_GENERIC));
}

//////////////////////////////////////////////////////////////////////////
// plWorld
//////////////////////////////////////////////////////////////////////////

static void plGameObjectDesc_Construct(void* pMemory)
{
  new (pMemory) plGameObjectDesc();
}

static void plWorld_TryGetComponent(asIScriptGeneric* pGen)
{
  plWorld* pWorld = (plWorld*)pGen->GetObject();
  plComponentHandle* hComponent = (plComponentHandle*)pGen->GetArgObject(0);
  int typeId = pGen->GetArgTypeId(1);

  if (auto info = pGen->GetEngine()->GetTypeInfoById(typeId))
  {
    if (const plRTTI* pRtti = static_cast<const plRTTI*>(info->GetUserData(plAsUserData::RttiPtr)))
    {
      plComponent* pComponent;
      if (pWorld->TryGetComponent(*hComponent, pComponent))
      {
        plComponent** ref = (plComponent**)pGen->GetArgAddress(1);
        *ref = pComponent;

        pGen->SetReturnByte((*ref)->GetDynamicRTTI()->IsDerivedFrom(pRtti) ? 1 : 0);
        return;
      }
    }
  }

  pGen->SetReturnByte(0);
}

static void plWorld_SendAsMessage(asIScriptGeneric* pGen)
{
  plWorld* pObj = (plWorld*)pGen->GetObject();
  plGameObjectHandle* pReceiver = (plGameObjectHandle*)pGen->GetArgObject(0);
  void* pAsMsg = pGen->GetArgObject(1);

  plMsgDeliverAngelScriptMsg msg;
  msg.m_pAsMsg = pAsMsg;
  pObj->SendMessage(*pReceiver, msg);
}

static void plWorld_SendAsMessageRecursive(asIScriptGeneric* pGen)
{
  plWorld* pObj = (plWorld*)pGen->GetObject();
  plGameObjectHandle* pReceiver = (plGameObjectHandle*)pGen->GetArgObject(0);
  void* pAsMsg = pGen->GetArgObject(1);

  plMsgDeliverAngelScriptMsg msg;
  msg.m_pAsMsg = pAsMsg;
  pObj->SendMessageRecursive(*pReceiver, msg);
}

static void plWorld_PostAsMessage(asIScriptGeneric* pGen)
{
  plWorld* pObj = (plWorld*)pGen->GetObject();
  plGameObjectHandle* pReceiver = (plGameObjectHandle*)pGen->GetArgObject(0);
  void* pAsMsg = pGen->GetArgObject(1);

  plTime delay = *((const plTime*)pGen->GetArgObject(2));
  plInt32 delivery = (plInt32)pGen->GetArgDWord(3);

  void* pMsgCopy = pGen->GetEngine()->CreateScriptObjectCopy(pAsMsg, reinterpret_cast<asIScriptObject*>(pAsMsg)->GetObjectType());
  PL_ASSERT_DEV(pMsgCopy != nullptr, "Failed to create copy of message");

  plMsgDeliverAngelScriptMsg msg;
  msg.m_bRelease = true;
  msg.m_pAsMsg = pMsgCopy;
  pObj->PostMessage(*pReceiver, msg, delay, static_cast<plObjectMsgQueueType::Enum>(delivery));
}

static void plWorld_PostAsMessageRecursive(asIScriptGeneric* pGen)
{
  plWorld* pObj = (plWorld*)pGen->GetObject();
  plGameObjectHandle* pReceiver = (plGameObjectHandle*)pGen->GetArgObject(0);
  void* pAsMsg = pGen->GetArgObject(1);

  plTime delay = *((const plTime*)pGen->GetArgObject(2));
  plInt32 delivery = (plInt32)pGen->GetArgDWord(3);

  void* pMsgCopy = pGen->GetEngine()->CreateScriptObjectCopy(pAsMsg, reinterpret_cast<asIScriptObject*>(pAsMsg)->GetObjectType());
  PL_ASSERT_DEV(pMsgCopy != nullptr, "Failed to create copy of message");

  plMsgDeliverAngelScriptMsg msg;
  msg.m_bRelease = true;
  msg.m_pAsMsg = pMsgCopy;
  pObj->PostMessageRecursive(*pReceiver, msg, delay, static_cast<plObjectMsgQueueType::Enum>(delivery));
}


void plAngelScriptEngineSingleton::Register_World()
{
  {
    AS_CHECK(m_pEngine->RegisterObjectType("plGameObjectDesc", sizeof(plGameObjectDesc), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<plGameObjectDesc>()));

    AS_CHECK(m_pEngine->RegisterObjectProperty("plGameObjectDesc", "bool m_bActiveFlag", asOFFSET(plGameObjectDesc, m_bActiveFlag)));
    AS_CHECK(m_pEngine->RegisterObjectProperty("plGameObjectDesc", "bool m_bDynamic", asOFFSET(plGameObjectDesc, m_bDynamic)));
    AS_CHECK(m_pEngine->RegisterObjectProperty("plGameObjectDesc", "uint16 m_uiTeamID", asOFFSET(plGameObjectDesc, m_uiTeamID)));
    AS_CHECK(m_pEngine->RegisterObjectProperty("plGameObjectDesc", "plHashedString m_sName", asOFFSET(plGameObjectDesc, m_sName)));
    AS_CHECK(m_pEngine->RegisterObjectProperty("plGameObjectDesc", "plGameObjectHandle m_hParent", asOFFSET(plGameObjectDesc, m_hParent)));
    AS_CHECK(m_pEngine->RegisterObjectProperty("plGameObjectDesc", "plVec3 m_LocalPosition", asOFFSET(plGameObjectDesc, m_LocalPosition)));
    AS_CHECK(m_pEngine->RegisterObjectProperty("plGameObjectDesc", "plQuat m_LocalRotation", asOFFSET(plGameObjectDesc, m_LocalRotation)));
    AS_CHECK(m_pEngine->RegisterObjectProperty("plGameObjectDesc", "plVec3 m_LocalScaling", asOFFSET(plGameObjectDesc, m_LocalScaling)));
    AS_CHECK(m_pEngine->RegisterObjectProperty("plGameObjectDesc", "float m_LocalUniformScaling", asOFFSET(plGameObjectDesc, m_LocalUniformScaling)));
    // AS_CHECK(m_pEngine->RegisterObjectProperty("plGameObjectDesc", "plTagSet m_Tags", asOFFSET(plGameObjectDesc, m_Tags)));
    AS_CHECK(m_pEngine->RegisterObjectProperty("plGameObjectDesc", "uint32 m_uiStableRandomSeed", asOFFSET(plGameObjectDesc, m_uiStableRandomSeed)));

    AS_CHECK(m_pEngine->RegisterObjectBehaviour("plGameObjectDesc", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(plGameObjectDesc_Construct), asCALL_CDECL_OBJFIRST));
  }

  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "plStringView GetName()", asMETHOD(plWorld, GetName), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "plGameObjectHandle CreateObject(const plGameObjectDesc& in desc)", asMETHODPR(plWorld, CreateObject, (const plGameObjectDesc& desc), plGameObjectHandle), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "plGameObjectHandle CreateObject(const plGameObjectDesc& in desc, plGameObject@& out object)", asMETHODPR(plWorld, CreateObject, (const plGameObjectDesc& desc, plGameObject*&), plGameObjectHandle), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "void DeleteObjectDelayed(const plGameObjectHandle& in hObject, bool bAlsoDeleteEmptyParents = true)", asMETHOD(plWorld, DeleteObjectDelayed), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "bool IsValidObject(const plGameObjectHandle& in hObject)", asMETHOD(plWorld, IsValidObject), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "bool TryGetObject(const plGameObjectHandle& in, plGameObject@& out pObject)", asMETHODPR(plWorld, TryGetObject, (const plGameObjectHandle&, plGameObject*&), bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "bool TryGetObject(const plGameObjectHandle& in, const plGameObject@& out pObject) const", asMETHODPR(plWorld, TryGetObject, (const plGameObjectHandle&, const plGameObject*&) const, bool), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "bool TryGetObjectWithGlobalKey(const plTempHashedString& in sGlobalKey, plGameObject@& out pObject)", asMETHODPR(plWorld, TryGetObjectWithGlobalKey, (const plTempHashedString& sGlobalKey, plGameObject*& out_pObject), bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "bool TryGetObjectWithGlobalKey(const plTempHashedString& in sGlobalKey, const plGameObject@& out pObject)", asMETHODPR(plWorld, TryGetObjectWithGlobalKey, (const plTempHashedString& sGlobalKey, const plGameObject*& out_pObject) const, bool), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "plGameObject@ SearchForObject(plStringView sSearchPath, plGameObject@ pStartSearchObj = null, const plRTTI@ pExpectedComponent = null)", asMETHODPR(plWorld, SearchForObject, (plStringView, plGameObject*, const plRTTI*), plGameObject*), asCALL_THISCALL));

  // GetOrCreateModule

  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "bool IsValidComponent(plComponentHandle& in)", asMETHOD(plWorld, IsValidComponent), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "bool TryGetComponent(const plComponentHandle& in hComponent, ?& out component)", asFUNCTION(plWorld_TryGetComponent), asCALL_GENERIC));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "void SendMessage(const plGameObjectHandle& in hReceiverObject, plMessage& inout msg)", asMETHODPR(plWorld, SendMessage, (const plGameObjectHandle&, plMessage&), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "void SendMessageRecursive(const plGameObjectHandle& in hReceiverObject, plMessage& inout msg)", asMETHODPR(plWorld, SendMessageRecursive, (const plGameObjectHandle&, plMessage&), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "void PostMessage(const plGameObjectHandle& in hReceiverObject, const plMessage& in msg, plTime delay, plObjectMsgQueueType queueType = plObjectMsgQueueType::NextFrame) const", asMETHODPR(plWorld, PostMessage, (const plGameObjectHandle&, const plMessage&, plTime, plObjectMsgQueueType::Enum) const, void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "void PostMessageRecursive(const plGameObjectHandle& in hReceiverObject, const plMessage& in msg, plTime delay, plObjectMsgQueueType queueType = plObjectMsgQueueType::NextFrame) const", asMETHODPR(plWorld, PostMessageRecursive, (const plGameObjectHandle&, const plMessage&, plTime, plObjectMsgQueueType::Enum) const, void), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "void SendMessage(const plComponentHandle& in hReceiverObject, plMessage& inout msg)", asMETHODPR(plWorld, SendMessage, (const plComponentHandle&, plMessage&), void), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "void PostMessage(const plComponentHandle& in hReceiverObject, const plMessage& in msg, plTime delay, plObjectMsgQueueType queueType = plObjectMsgQueueType::NextFrame) const", asMETHODPR(plWorld, PostMessage, (const plComponentHandle&, const plMessage&, plTime, plObjectMsgQueueType::Enum) const, void), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "plClock@ GetClock()", asMETHODPR(plWorld, GetClock, (), plClock&), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "const plClock@ GetClock() const", asMETHODPR(plWorld, GetClock, () const, const plClock&),
    asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "plRandom@ GetRandomNumberGenerator()", asMETHOD(plWorld, GetRandomNumberGenerator), asCALL_THISCALL));

  // AS messages
  {
    AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "void SendMessage(const plGameObjectHandle& in hReceiverObject, plAngelScriptMessage& inout)", asFUNCTION(plWorld_SendAsMessage), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "void SendMessageRecursive(const plGameObjectHandle& in hReceiverObject, plAngelScriptMessage& inout)", asFUNCTION(plWorld_SendAsMessageRecursive), asCALL_GENERIC));

    AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "void PostMessage(const plGameObjectHandle& in hReceiverObject, const plAngelScriptMessage& in, plTime delay, plObjectMsgQueueType delivery = plObjectMsgQueueType::NextFrame)", asFUNCTION(plWorld_PostAsMessage), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plWorld", "void PostMessageRecursive(const plGameObjectHandle& in hReceiverObject, const plAngelScriptMessage& in, plTime delay, plObjectMsgQueueType delivery = plObjectMsgQueueType::NextFrame)", asFUNCTION(plWorld_PostAsMessageRecursive), asCALL_GENERIC));
  }
}
