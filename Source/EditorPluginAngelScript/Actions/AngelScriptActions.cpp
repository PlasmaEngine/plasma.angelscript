#include <EditorPluginAngelScript/EditorPluginAngelScriptPCH.h>

#include <EditorPluginAngelScript/Actions/AngelScriptActions.h>
#include <EditorPluginAngelScript/AngelScriptAsset/AngelScriptAsset.h>
#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMapManager.h>

// clang-format off
PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plAngelScriptAction, 1, plRTTINoAllocator)
PL_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

plActionDescriptorHandle plAngelScriptActions::s_hCategory;
plActionDescriptorHandle plAngelScriptActions::s_hOpenInVSC;
plActionDescriptorHandle plAngelScriptActions::s_hSyncExposedParams;

void plAngelScriptActions::RegisterActions()
{
  s_hCategory = PL_REGISTER_CATEGORY("AngelScriptCategory");
  s_hOpenInVSC = PL_REGISTER_ACTION_1("AngelScript.OpenInVSC", plActionScope::Document, "AngelScript", "", plAngelScriptAction, plAngelScriptAction::ActionType::OpenInVSC);
  s_hSyncExposedParams = PL_REGISTER_ACTION_1("AngelScript.SyncExposedParams", plActionScope::Document, "AngelScript", "", plAngelScriptAction, plAngelScriptAction::ActionType::SyncExposedParameters);
}

void plAngelScriptActions::UnregisterActions()
{
  plActionManager::UnregisterAction(s_hCategory);
  plActionManager::UnregisterAction(s_hOpenInVSC);
  plActionManager::UnregisterAction(s_hSyncExposedParams);
}

void plAngelScriptActions::MapActions(plStringView sMapping)
{
  plActionMap* pMap = plActionMapManager::GetActionMap(sMapping);
  PL_ASSERT_DEV(pMap != nullptr, "The given mapping ('{0}') does not exist, mapping the actions failed!", sMapping);

  pMap->MapAction(s_hCategory, "", 11.0f);

  const char* szSubPath = "AngelScriptCategory";

  pMap->MapAction(s_hOpenInVSC, szSubPath, 1.0f);
  pMap->MapAction(s_hSyncExposedParams, szSubPath, 2.0f);
}

plAngelScriptAction::plAngelScriptAction(const plActionContext& context, const char* szName, plAngelScriptAction::ActionType type)
  : plButtonAction(context, szName, false, "")
{
  m_Type = type;

  m_pDocument = const_cast<plAngelScriptAssetDocument*>(static_cast<const plAngelScriptAssetDocument*>(context.m_pDocument));

  switch (m_Type)
  {
    case ActionType::OpenInVSC:
      SetIconPath(":/GuiFoundation/Icons/vscode.svg");
      break;
    case ActionType::SyncExposedParameters:
      SetIconPath(":/GuiFoundation/Icons/ReloadResources.svg");
      break;
  }
}


void plAngelScriptAction::Execute(const plVariant& value)
{
  switch (m_Type)
  {
    case ActionType::OpenInVSC:
      m_pDocument->OpenExternalEditor();
      return;
    case ActionType::SyncExposedParameters:
      m_pDocument->SyncExposedParameters();
      return;
  }
}
