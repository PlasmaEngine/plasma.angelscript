#pragma once

#include <AngelScriptPlugin/AngelScriptPluginDLL.h>
#include <Core/Scripting/ScriptRTTI.h>
#include <Foundation/Communication/Message.h>
#include <Foundation/Strings/String.h>

//////////////////////////////////////////////////////////////////////////

class asIScriptFunction;

class PL_ANGELSCRIPTPLUGIN_DLL plAngelScriptFunctionProperty : public plScriptFunctionProperty
{
public:
  plAngelScriptFunctionProperty(plStringView sName, asIScriptFunction* pFunction);
  ~plAngelScriptFunctionProperty();

  virtual plFunctionType::Enum GetFunctionType() const override { return plFunctionType::Member; }
  virtual const plRTTI* GetReturnType() const override { return nullptr; }
  virtual plBitflags<plPropertyFlags> GetReturnFlags() const override { return plPropertyFlags::Void; }
  virtual plUInt32 GetArgumentCount() const override { return 0; }
  virtual const plRTTI* GetArgumentType(plUInt32 uiParamIndex) const override { return nullptr; }
  virtual plBitflags<plPropertyFlags> GetArgumentFlags(plUInt32 uiParamIndex) const override { return plPropertyFlags::Void; }

  virtual void Execute(void* pInstance, plArrayPtr<plVariant> arguments, plVariant& out_returnValue) const override;

private:
  asIScriptFunction* m_pAsFunction = nullptr;
};

//////////////////////////////////////////////////////////////////////////

class PL_ANGELSCRIPTPLUGIN_DLL plAngelScriptMessageHandler : public plScriptMessageHandler
{
public:
  plAngelScriptMessageHandler(const plScriptMessageDesc& desc, asIScriptFunction* pFunction);
  ~plAngelScriptMessageHandler();

  static void Dispatch(plAbstractMessageHandler* pSelf, void* pInstance, plMessage& ref_msg);

private:
  asIScriptFunction* m_pAsFunction = nullptr;
};

//////////////////////////////////////////////////////////////////////////

struct plMsgDeliverAngelScriptMsg : public plMessage
{
  PL_DECLARE_MESSAGE_TYPE(plMsgDeliverAngelScriptMsg, plMessage);

  ~plMsgDeliverAngelScriptMsg();

  plMsgDeliverAngelScriptMsg(const plMsgDeliverAngelScriptMsg& rhs);
  plMsgDeliverAngelScriptMsg(plMsgDeliverAngelScriptMsg&& rhs);
  void operator=(const plMsgDeliverAngelScriptMsg& rhs);
  void operator=(plMsgDeliverAngelScriptMsg&& rhs);

  bool m_bRelease = false;
  void* m_pAsMsg = nullptr;
};

class PL_ANGELSCRIPTPLUGIN_DLL plAngelScriptCustomAsMessageHandler : public plScriptMessageHandler
{
public:
  plAngelScriptCustomAsMessageHandler(const plScriptMessageDesc& desc);
  ~plAngelScriptCustomAsMessageHandler();

  void AddReceiver(asIScriptFunction* pFunction, const char* szArgType);

  static void Dispatch(plAbstractMessageHandler* pSelf, void* pInstance, plMessage& ref_msg);

private:
  struct Receiver
  {
    plHashedString m_sArgType;
    asIScriptFunction* m_pAsFunction = nullptr;
  };

  plHybridArray<Receiver, 2> m_Receivers;
};
