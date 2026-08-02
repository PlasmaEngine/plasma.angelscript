#pragma once

#include <AngelScriptPlugin/AngelScriptPluginDLL.h>
#include <Core/Scripting/ScriptClassResource.h>
#include <Core/Scripting/ScriptRTTI.h>
#include <Foundation/Communication/Message.h>
#include <Foundation/Containers/Blob.h>

class asIScriptContext;
class asIScriptModule;
class asIScriptObject;
class plComponent;
class plScriptComponent;

class PL_ANGELSCRIPTPLUGIN_DLL plAngelScriptInstance : public plScriptInstance
{
public:
  plAngelScriptInstance(plReflectedClass& inout_owner, plWorld* pWorld, asIScriptModule* pModule, const char* szObjectTypeName);
  ~plAngelScriptInstance();

  virtual void SetInstanceVariable(const plHashedString& sName, const plVariant& value) override;
  virtual plVariant GetInstanceVariable(const plHashedString& sName) override;

  asIScriptContext* GetContext() const { return m_pContext; }
  asIScriptObject* GetObject() const { return m_pObject; }
  plScriptComponent* GetOwnerComponent() const { return m_pOwnerComponent; }

private:
  void ExceptionCallback(asIScriptContext* pContext);

  asIScriptObject* m_pObject = nullptr;
  asIScriptContext* m_pContext = nullptr;
  plScriptComponent* m_pOwnerComponent = nullptr;
};
