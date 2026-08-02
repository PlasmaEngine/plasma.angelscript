#pragma once

#include <EditorPluginAngelScript/EditorPluginAngelScriptDLL.h>

#include <GuiFoundation/Action/BaseActions.h>
#include <GuiFoundation/GuiFoundationDLL.h>

class plAngelScriptAssetDocument;
struct plAngelScriptAssetEvent;

class plAngelScriptActions
{
public:
  static void RegisterActions();
  static void UnregisterActions();

  static void MapActions(plStringView sMapping);

  static plActionDescriptorHandle s_hCategory;
  static plActionDescriptorHandle s_hOpenInVSC;
  static plActionDescriptorHandle s_hSyncExposedParams;
};

class plAngelScriptAction : public plButtonAction
{
  PL_ADD_DYNAMIC_REFLECTION(plAngelScriptAction, plButtonAction);

public:
  enum class ActionType
  {
    OpenInVSC,
    SyncExposedParameters,
  };

  plAngelScriptAction(const plActionContext& context, const char* szName, ActionType type);

  virtual void Execute(const plVariant& value) override;

private:
  plAngelScriptAssetDocument* m_pDocument = nullptr;
  ActionType m_Type;
};
