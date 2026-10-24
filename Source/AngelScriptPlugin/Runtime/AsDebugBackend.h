#pragma once

#include <AngelScriptPlugin/AngelScriptPluginDLL.h>
#include <Core/Scripting/ScriptDebugger.h>

class asIScriptContext;
class plAngelScriptInstance;

/// \brief AngelScript's part of the script debugger: a line callback on the contexts of all instances while a session is
/// attached, and frames and variables read from those contexts while stopped.
class PL_ANGELSCRIPTPLUGIN_DLL plAngelScriptDebugBackend : public plScriptDebugBackend
{
public:
  plAngelScriptDebugBackend();
  ~plAngelScriptDebugBackend();

  static plStringView GetLanguageName() { return "AngelScript"; }

  virtual plStringView GetLanguage() const override { return GetLanguageName(); }
  virtual void OnDebugStateChanged() override;
  virtual void CollectFrames(const plScriptCallFrame& entry, plDynamicArray<plScriptDebugFrame>& out_frames) override;
  virtual void GetVariables(const plScriptCallFrame& entry, plUInt32 uiLocalFrame, plDynamicArray<plScriptDebugVariable>& out_variables) override;
  virtual plResult SetVariable(const plScriptCallFrame& entry, plUInt32 uiLocalFrame, plStringView sName, plStringView sValue) override;

  /// \brief Installs or removes the line callback on one instance's context, depending on the session state.
  void UpdateContext(plAngelScriptInstance* pInstance);

  /// \brief The original file and line of a stack level of a context, through the module's source map.
  static plScriptDebugLocation GetLocation(asIScriptContext* pContext, plUInt32 uiStackLevel);

private:
  static void LineCallback(asIScriptContext* pContext, void* pParam);

  bool m_bHooked = false;
};
