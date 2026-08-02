#pragma once

#include <EditorFramework/DocumentWindow/EngineDocumentWindow.moc.h>
#include <Foundation/Basics.h>
#include <GuiFoundation/DocumentWindow/DocumentWindow.moc.h>
#include <QTimer>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

class plAngelScriptAssetDocument;
struct plAssetCuratorEvent;
class plEditorEngineDocumentMsg;
class QTextEdit;

class plQtAngelScriptAssetDocumentWindow : public plQtEngineDocumentWindow
{
  Q_OBJECT

public:
  plQtAngelScriptAssetDocumentWindow(plAngelScriptAssetDocument* pDocument);
  ~plQtAngelScriptAssetDocumentWindow();

private Q_SLOTS:
  void onTextEditTextChanged();
  void onEditTimer();

private:
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
  QTextEdit* m_pSourceLabel = nullptr;
  QSyntaxHighlighter* m_pHighlighter = nullptr;
  QTimer m_EditTimer;
  plTime m_LastEdit;
  plTime m_LastSave;
};


//////////////////////////////////////////////////////////////////////////

#include <QSyntaxHighlighter>

struct ASEdit
{
  enum ColorComponent
  {
    Comment,
    Number,
    String,
    Operator,
    KeywordBlue,
    KeywordPink,
    KeywordGreen,
    BuiltIn,

    Count,
  };
};

class ASBlockData : public QTextBlockUserData
{
public:
  QList<int> bracketPositions;
};

class ASHighlighter : public QSyntaxHighlighter
{
public:
  ASHighlighter(QTextDocument* pParent = 0);

protected:
  void highlightBlock(const QString& text) override;

private:
  QColor m_Colors[ASEdit::Count];
};
