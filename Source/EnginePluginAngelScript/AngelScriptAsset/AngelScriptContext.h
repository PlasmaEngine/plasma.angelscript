#pragma once

#include <EditorEngineProcessFramework/EngineProcess/EngineProcessDocumentContext.h>
#include <EnginePluginAngelScript/EnginePluginAngelScriptDLL.h>

class PL_ENGINEPLUGINAS_DLL plAngelScriptDocumentContext : public plEngineProcessDocumentContext
{
  PL_ADD_DYNAMIC_REFLECTION(plAngelScriptDocumentContext, plEngineProcessDocumentContext);

public:
  plAngelScriptDocumentContext();
  ~plAngelScriptDocumentContext();

protected:
  virtual plStatus ExportDocument(const plExportDocumentMsgToEngine* pMsg) override;
  virtual void HandleMessage(const plEditorEngineDocumentMsg* pMsg) override;

  plEngineProcessViewContext* CreateViewContext() override;
  void DestroyViewContext(plEngineProcessViewContext* pContext) override;

  void SyncExposedParameters();
  void CheckCode();
  asIScriptModule* CompileModule(plStringBuilder& out_sCode, plSet<plString>* out_pDependencies);
  void RetrieveScriptInfos(plStringView sBasePath);

  plString m_sInputFile;
  plString m_sClass;
  plString m_sCode;
};
