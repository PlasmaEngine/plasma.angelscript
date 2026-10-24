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

  virtual plStringView GetScriptLanguage() const override { return "AngelScript"; }

  virtual void SetInstanceVariable(const plHashedString& sName, const plVariant& value) override;
  virtual plVariant GetInstanceVariable(const plHashedString& sName) override;

  /// \brief Writes the values that match a member; names without a member and values that do not convert are skipped,
  /// because reload state from an older version of the script may name members that are gone.
  virtual void SetInstanceVariables(const plArrayMap<plHashedString, plVariant>& parameters) override;

  /// \brief Captures every member that converts to a variant, so a reload can restore private state too.
  virtual void GetInstanceVariables(plArrayMap<plHashedString, plVariant>& out_parameters) const override;

  asIScriptContext* GetContext() const { return m_pContext; }
  asIScriptObject* GetObject() const { return m_pObject; }
  plScriptComponent* GetOwnerComponent() const { return m_pOwnerComponent; }

private:
  void ExceptionCallback(asIScriptContext* pContext);
  plResult ReadMember(plUInt32 uiIndex, plVariant& out_value) const;

  asIScriptObject* m_pObject = nullptr;
  asIScriptContext* m_pContext = nullptr;
  plScriptComponent* m_pOwnerComponent = nullptr;
};
