#pragma once

#include <EditorFramework/DocumentWindow/EngineDocumentWindow.h>
#include <EditorUi/Content.h>
#include <EditorUi/Runtime.h>
#include <Foundation/Basics.h>
#include <GameUiPlugin/Elements/UiElements.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

class plAngelScriptAssetDocument;
struct plAssetCuratorEvent;
class plEditorEngineDocumentMsg;

/// The script view: an engine-rendered code area in AngelScript colours. Inline code is
/// editable; a script file is shown read-only.
///
/// The view is an editor UI content: the window's "Script" panel, which the engine shell shows in the document's frame.
class plAngelScriptCodeView : public plEditorUiContent
{
public:
  plAngelScriptCodeView();
  ~plAngelScriptCodeView();

  void SetCode(plStringView sCode, bool bReadOnly);
  plStringView GetCode() const { return m_sCode; }

  /// The code area's element name; decorations are looked up by it, so it must be unique per window.
  void SetAreaName(plStringView sName);

  /// Markers, breakpoints and the stopped line of the code area.
  void SetDecorations(const plUiCodeArea::Decorations& decorations);

  /// Called when the user changed the code.
  plDelegate<void()> m_OnEdited;

  /// Called with the 1-based line when the user clicks the line number gutter.
  plDelegate<void(plUInt32)> m_OnGutterClicked;

  /// Registers the "AngelScript" language for code areas; the keyword lists grow with the
  /// project's script infos (types, functions) once they were retrieved.
  static void RegisterLanguage();

protected:
  void OnUiUpdate() override;
  void OnUiEvent(const plMsgUiEvent& event) override;

  /// The description of the member or engine type under the pointer.
  plString GetToolTipAt(const plVec2& vLogical) const override;

private:
  void BuildDocument();

  plString m_sCode;
  plString m_sAreaName = "Script.area";
  bool m_bReadOnly = true;
  bool m_bRebuild = true;
};

class plAngelScriptAssetDocumentWindow : public plEngineDocumentWindow
{
public:
  plAngelScriptAssetDocumentWindow(plAngelScriptAssetDocument* pDocument);
  ~plAngelScriptAssetDocumentWindow();

  /// Asks the window of a document to show a line once it is open (or right away when it is). Used to open problems.
  static void RequestReveal(const plUuid& documentGuid, plUInt32 uiLine);

  /// The key breakpoints and locations of this script use: the data directory relative document path for inline code,
  /// the script file for file-based scripts.
  static plString GetLocationKey(const plAngelScriptAssetDocument* pDocument);

private:
  void UpdateDecorations();
  void OnScriptToolsEvent(const struct plScriptToolsEvent& e);
  void OnGutterClicked(plUInt32 uiLine);
  void RequestCheck();
  void OnEditTimer();
  void OnCodeEdited();
  void AssetEventHandler(const plAssetCuratorEvent& e);
  void UpdateFileContentDisplay();
  virtual void ProcessMessageEventHandler(const plEditorEngineDocumentMsg* pMsg) override;
  void RetrieveScriptInfos();
  void DocumentObjectEventHandler(const plDocumentObjectPropertyEvent& e);
  void StoreInlineDocState();

  struct ExposedParam
  {
    plString m_sName;
    plVariant m_DefaultValue;
    bool m_bExpose = false;
  };

  plDynamicArray<ExposedParam> m_ExposedParams;
  plDynamicArray<plString> m_Dependencies;

  plAngelScriptAssetDocument* m_pAssetDoc = nullptr;

  bool m_bIgnoreCodeChange = false;
  plAngelScriptCodeView* m_pCodeView = nullptr;
  plEditorUiTimerGroup m_Timers;
  plUInt32 m_uiEditTimer = 0;
  plTime m_LastEdit;
  plTime m_LastSave;
  plTime m_LastCheck;
  plUInt32 m_uiHighlightLine = 0;
  plEventSubscriptionID m_ToolsSubscription = 0;
};
