#include <EditorPluginAssets/EditorPluginAssetsPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Runtime/AsEngineSingleton.h>
#include <AngelScriptPlugin/Utils/AngelScriptUtils.h>
#include <Core/Scripting/ScriptApiDocs.h>
#include <EditorFramework/Assets/AssetCurator.h>
#include <EditorFramework/EditorApp/EditorApp.h>
#include <EditorFramework/Scripting/ScriptTools.h>
#include <Foundation/Utilities/ConversionUtils.h>
#include <EditorPluginAngelScript/AngelScriptAsset/AngelScriptAsset.h>
#include <EditorPluginAngelScript/AngelScriptWindow/AngelScriptWindow.h>
#include <EditorUi/Surface.h>
#include <EditorUi/Widgets.h>
#include <GameUiPlugin/Components/UiCanvasComponent.h>
#include <GameUiPlugin/Elements/UiElements.h>
#include <ToolsFoundation/Application/ApplicationServices.h>
#include <ToolsFoundation/Object/ObjectCommandAccessor.h>

namespace
{
  constexpr const char* s_szLanguage = "AngelScript";
  constexpr const char* s_szCodeBinding = "Script.code";

  bool g_bRetrievedScriptInfos = false;

  // Words the project's script infos add: types and namespaces, and functions, methods, enums
  // and properties. Space-separated.
  plStringBuilder g_sProjectTypes;
  plStringBuilder g_sProjectBuiltIns;
} // namespace

//////////////////////////////////////////////////////////////////////////
// Completion

namespace
{
  /// The description shown under the completion list: the engine's docs for the reflected member when there are any.
  plString DescribeMember(plStringView sTypeName, plStringView sMember, plStringView sDeclaration)
  {
    plScriptApiDocs::EnsureLoaded();
    plStringBuilder sDoc = plScriptApiDocs::GetMemberDescription(sTypeName, sMember);
    if (sDoc.IsEmpty())
      sDoc = sDeclaration;
    return sDoc;
  }

  void AddTypeMembers(asIScriptEngine* pEngine, const asITypeInfo* pType, plStringView sPrefix, plDynamicArray<plUiCodeCompletionItem>& out_items)
  {
    plSet<plString> seen;

    for (asUINT i = 0; i < pType->GetMethodCount(); ++i)
    {
      const asIScriptFunction* pFunc = pType->GetMethodByIndex(i);
      const plStringView sName = pFunc->GetName();
      if (!sName.StartsWith_NoCase(sPrefix) || sName.StartsWith("op") || seen.Contains(sName))
        continue;

      seen.Insert(sName);
      plUiCodeCompletionItem& item = out_items.ExpandAndGetRef();
      item.m_sLabel = sName;
      item.m_sInsert = plStringBuilder(sName, "(");
      item.m_sDetail = pFunc->GetDeclaration(false, false, true);
      item.m_sDoc = DescribeMember(pType->GetName(), sName, item.m_sDetail);
    }

    for (asUINT i = 0; i < pType->GetPropertyCount(); ++i)
    {
      const char* szName = nullptr;
      int iTypeId = 0;
      pType->GetProperty(i, &szName, &iTypeId);
      if (szName == nullptr || !plStringView(szName).StartsWith_NoCase(sPrefix) || seen.Contains(szName))
        continue;

      seen.Insert(szName);
      plUiCodeCompletionItem& item = out_items.ExpandAndGetRef();
      item.m_sLabel = szName;
      item.m_sInsert = szName;
      item.m_sDetail = pEngine->GetTypeDeclaration(iTypeId, true);
      item.m_sDoc = DescribeMember(pType->GetName(), szName, item.m_sDetail);
    }
  }

  void AddNamespaceMembers(asIScriptEngine* pEngine, plStringView sNamespace, plStringView sPrefix, plDynamicArray<plUiCodeCompletionItem>& out_items)
  {
    plSet<plString> seen;

    for (asUINT i = 0; i < pEngine->GetGlobalFunctionCount(); ++i)
    {
      const asIScriptFunction* pFunc = pEngine->GetGlobalFunctionByIndex(i);
      if (sNamespace != pFunc->GetNamespace())
        continue;

      const plStringView sName = pFunc->GetName();
      if (!sName.StartsWith_NoCase(sPrefix) || seen.Contains(sName))
        continue;

      seen.Insert(sName);
      plUiCodeCompletionItem& item = out_items.ExpandAndGetRef();
      item.m_sLabel = sName;
      item.m_sInsert = plStringBuilder(sName, "(");
      item.m_sDetail = pFunc->GetDeclaration(false, false, true);
      item.m_sDoc = DescribeMember(sNamespace, sName, item.m_sDetail);
    }

    // Enum values of an enum named like the namespace.
    for (asUINT i = 0; i < pEngine->GetEnumCount(); ++i)
    {
      const asITypeInfo* pEnum = pEngine->GetEnumByIndex(i);
      if (sNamespace != pEnum->GetName())
        continue;

      for (asUINT v = 0; v < pEnum->GetEnumValueCount(); ++v)
      {
        int iValue = 0;
        const plStringView sName = pEnum->GetEnumValueByIndex(v, &iValue);
        if (!sName.StartsWith_NoCase(sPrefix))
          continue;

        plUiCodeCompletionItem& item = out_items.ExpandAndGetRef();
        item.m_sLabel = sName;
        item.m_sInsert = sName;
        plStringBuilder sDetail;
        sDetail.SetFormat("= {}", iValue);
        item.m_sDetail = sDetail;
      }
    }
  }

  /// The type of an identifier from its declaration in the text ("Type name", "Type@ name", a parameter).
  plString FindDeclaredType(plStringView sText, plStringView sName)
  {
    plStringBuilder sTypeName;

    const char* szSearch = sText.GetEndPointer();
    while (true)
    {
      const char* szFound = sText.FindLastSubString(sName, szSearch);
      if (szFound == nullptr)
        return {};

      szSearch = szFound;

      // whole word, followed by an initializer, a separator or the end of a parameter
      const char* szAfter = szFound + sName.GetElementCount();
      if (szAfter < sText.GetEndPointer() && (plStringUtils::IsIdentifierDelimiter_C_Code(*szAfter) == false))
        continue;
      if (szFound > sText.GetStartPointer() && plStringUtils::IsIdentifierDelimiter_C_Code(*(szFound - 1)) == false)
        continue;

      // the word before it, skipping spaces and a handle marker
      const char* szPos = szFound;
      while (szPos > sText.GetStartPointer() && (*(szPos - 1) == ' ' || *(szPos - 1) == '\t'))
        --szPos;
      if (szPos > sText.GetStartPointer() && *(szPos - 1) == '@')
        --szPos;
      const char* szEnd = szPos;
      while (szPos > sText.GetStartPointer() && !plStringUtils::IsIdentifierDelimiter_C_Code(*(szPos - 1)))
        --szPos;

      if (szPos == szEnd)
        continue;

      sTypeName = plStringView(szPos, szEnd);
      if (sTypeName == "return" || sTypeName == "new" || sTypeName == "else")
        continue;

      return sTypeName;
    }
  }

  /// Completion for "a.b.", "Namespace::" and "GetOwner()." before the caret.
  void CompleteAngelScript(plStringView sBefore, plDynamicArray<plUiCodeCompletionItem>& out_items, plUInt32& out_uiReplaceLength)
  {
    plAngelScriptEngineSingleton* pSingleton = plAngelScriptEngineSingleton::GetSingleton();
    if (pSingleton == nullptr)
      return;

    asIScriptEngine* pEngine = pSingleton->GetEngine();

    // the identifier being typed
    const char* szWordStart = sBefore.GetEndPointer();
    while (szWordStart > sBefore.GetStartPointer() && !plStringUtils::IsIdentifierDelimiter_C_Code(*(szWordStart - 1)))
      --szWordStart;

    const plStringView sPrefix(szWordStart, sBefore.GetEndPointer());
    out_uiReplaceLength = sPrefix.GetElementCount();

    const plStringView sHead(sBefore.GetStartPointer(), szWordStart);

    if (sHead.EndsWith("::"))
    {
      const char* szNs = sHead.GetEndPointer() - 2;
      const char* szNsStart = szNs;
      while (szNsStart > sHead.GetStartPointer() && !plStringUtils::IsIdentifierDelimiter_C_Code(*(szNsStart - 1)))
        --szNsStart;

      const plStringView sNamespace(szNsStart, szNs);
      AddNamespaceMembers(pEngine, sNamespace, sPrefix, out_items);
      return;
    }

    if (!sHead.EndsWith("."))
      return;

    // The chain before the dot: identifiers and calls separated by dots, e.g. "GetOwner().GetParent()".
    plHybridArray<plStringView, 4> chain;
    const char* szEnd = sHead.GetEndPointer() - 1;
    const char* szPos = szEnd;
    while (szPos > sHead.GetStartPointer())
    {
      const char c = *(szPos - 1);
      if (c == ')')
      {
        // skip a call's arguments
        int iDepth = 0;
        while (szPos > sHead.GetStartPointer())
        {
          const char d = *(--szPos);
          if (d == ')')
            ++iDepth;
          else if (d == '(' && --iDepth == 0)
            break;
        }
        continue;
      }
      if (c == '.')
      {
        chain.PushBack(plStringView(szPos, szEnd));
        szEnd = szPos - 1;
        --szPos;
        continue;
      }
      if (plStringUtils::IsIdentifierDelimiter_C_Code(c))
        break;
      --szPos;
    }
    chain.PushBack(plStringView(szPos, szEnd));

    // Resolve from the left: the first element is a variable, a member of the script's base class, or a call of one.
    const asITypeInfo* pType = nullptr;
    for (plUInt32 i = chain.GetCount(); i > 0; --i)
    {
      plStringView sPart = chain[i - 1];
      const bool bCall = sPart.EndsWith(")");
      if (bCall)
      {
        const char* szParen = sPart.FindSubString("(");
        sPart = plStringView(sPart.GetStartPointer(), szParen != nullptr ? szParen : sPart.GetEndPointer());
      }

      plStringBuilder sName = sPart;
      sName.Trim(" \t");

      if (pType == nullptr && i == chain.GetCount())
      {
        if (bCall)
        {
          // a method of plAngelScriptClass, e.g. GetOwner()
          if (const asITypeInfo* pBase = pEngine->GetTypeInfoByName("plAngelScriptClass"))
          {
            for (asUINT m = 0; m < pBase->GetMethodCount(); ++m)
            {
              if (sName == pBase->GetMethodByIndex(m)->GetName())
              {
                pType = pEngine->GetTypeInfoById(pBase->GetMethodByIndex(m)->GetReturnTypeId());
                break;
              }
            }
          }
        }
        else
        {
          const plString sTypeName = FindDeclaredType(plStringView(sBefore.GetStartPointer(), sHead.GetEndPointer()), sName);
          if (!sTypeName.IsEmpty())
            pType = pEngine->GetTypeInfoByName(sTypeName);
        }
      }
      else if (pType != nullptr)
      {
        const asITypeInfo* pNext = nullptr;
        if (bCall)
        {
          for (asUINT m = 0; m < pType->GetMethodCount() && pNext == nullptr; ++m)
          {
            if (sName == pType->GetMethodByIndex(m)->GetName())
              pNext = pEngine->GetTypeInfoById(pType->GetMethodByIndex(m)->GetReturnTypeId());
          }
        }
        else
        {
          for (asUINT p = 0; p < pType->GetPropertyCount() && pNext == nullptr; ++p)
          {
            const char* szName = nullptr;
            int iTypeId = 0;
            pType->GetProperty(p, &szName, &iTypeId);
            if (szName != nullptr && sName == szName)
              pNext = pEngine->GetTypeInfoById(iTypeId);
          }
        }
        pType = pNext;
      }

      if (pType == nullptr)
        return;
    }

    if (pType != nullptr)
      AddTypeMembers(pEngine, pType, sPrefix, out_items);
  }
} // namespace

//////////////////////////////////////////////////////////////////////////
// Code view

plAngelScriptCodeView::plAngelScriptCodeView()
  : plEditorUiContent("AngelScript.Code")
{
  RegisterLanguage();
}

plAngelScriptCodeView::~plAngelScriptCodeView()
{
  plUiCodeArea::ClearDecorations(m_sAreaName);
}

void plAngelScriptCodeView::SetAreaName(plStringView sName)
{
  plUiCodeArea::ClearDecorations(m_sAreaName);
  m_sAreaName = sName;
  m_bRebuild = true;
}

void plAngelScriptCodeView::SetDecorations(const plUiCodeArea::Decorations& decorations)
{
  plUiCodeArea::SetDecorations(m_sAreaName, decorations);
  RequestRedraw();
}

plString plAngelScriptCodeView::GetToolTipAt(const plVec2& vLogical) const
{
  // Hover documentation: the member or type under the pointer, described as completion describes it.
  plString sElement;
  plString sText;
  plInt32 iIndex = -1;
  if (GetSurface() == nullptr || !GetSurface()->GetCanvas().GetHoveredCodePosition(sElement, sText, iIndex) || sElement != m_sAreaName)
    return plEditorUiContent::GetToolTipAt(vLogical);

  // the caret index counts characters; find the identifier around it
  const char* szPos = sText.GetData();
  for (plInt32 i = 0; i < iIndex && *szPos != '\0'; ++i)
    plUnicodeUtils::MoveToNextUtf8(szPos).IgnoreResult();

  const char* szStart = szPos;
  while (szStart > sText.GetData() && !plStringUtils::IsIdentifierDelimiter_C_Code(*(szStart - 1)))
    --szStart;
  const char* szEnd = szPos;
  while (*szEnd != '\0' && !plStringUtils::IsIdentifierDelimiter_C_Code(*szEnd))
    ++szEnd;

  const plStringView sWord(szStart, szEnd);
  if (sWord.IsEmpty())
    return {};

  plDynamicArray<plUiCodeCompletionItem> items;
  plUInt32 uiReplace = 0;
  CompleteAngelScript(plStringView(sText.GetData(), szEnd), items, uiReplace);
  for (const plUiCodeCompletionItem& item : items)
  {
    if (item.m_sLabel != sWord)
      continue;

    plStringBuilder sTip(item.m_sDetail);
    if (!item.m_sDoc.IsEmpty())
      sTip.Append(sTip.IsEmpty() ? "" : "\n", item.m_sDoc);
    return sTip;
  }

  plScriptApiDocs::EnsureLoaded();
  return plScriptApiDocs::GetTypeDescription(sWord);
}

void plAngelScriptCodeView::OnUiEvent(const plMsgUiEvent& event)
{
  // "<area>.gutter" with the clicked line as the item index
  const plStringView sId = event.m_sIdentifier.GetView();
  if (sId.EndsWith(".gutter") && sId.StartsWith(m_sAreaName) && event.m_iItemIndex > 0 && m_OnGutterClicked.IsValid())
  {
    m_OnGutterClicked(static_cast<plUInt32>(event.m_iItemIndex));
  }
}

void plAngelScriptCodeView::RegisterLanguage()
{
  plUiCodeLanguage language;
  language.m_sName = s_szLanguage;
  language.m_CommentColor = plColorGammaUB(0x6A, 0x8A, 0x35);
  language.m_NumberColor = plColorGammaUB(0xB5, 0xCE, 0xA8);
  language.m_StringColor = plColorGammaUB(0xCE, 0x91, 0x6A);

  // Control flow, declarations, types, then the project's functions.
  language.AddKeywords(plColorGammaUB(0xC5, 0x86, 0xC0), "break case continue default do for return switch while if else in out inout is plAngelScriptClass");
  language.AddKeywords(plColorGammaUB(0x56, 0x9C, 0xCA),
    "function funcdef import cast this void true false null class struct const enum private protected auto explicit external final namespace "
    "interface mixin abstract not and or xor override property shared super try catch typedef");

  plStringBuilder types("bool float double int int8 int16 int32 int64 uint8 uint16 uint32 uint64 ", g_sProjectTypes);
  language.AddKeywords(plColorGammaUB(0x4E, 0xC9, 0xB0), types);
  language.AddKeywords(plColorGammaUB(0xDC, 0xDC, 0xAA), g_sProjectBuiltIns);

  // Members after "." and "::", from the engine's own registrations (reflection plus the script API).
  language.m_CompletionProvider = plUiCodeLanguage::CompletionProvider(&CompleteAngelScript);

  plUiCodeLanguage::Register(language);
}

void plAngelScriptCodeView::SetCode(plStringView sCode, bool bReadOnly)
{
  m_sCode = sCode;

  if (IsReady())
    GetSurface()->GetBlackboard().SetEntryValue(s_szCodeBinding, m_sCode);

  if (m_bReadOnly != bReadOnly)
  {
    m_bReadOnly = bReadOnly;
    m_bRebuild = true;
  }
}

void plAngelScriptCodeView::OnUiUpdate()
{
  if (m_bRebuild)
    BuildDocument();

  // The area writes its binding while the user types.
  const plString shown = GetSurface()->GetBlackboard().GetEntryValue(plTempHashedString(s_szCodeBinding), m_sCode).ConvertTo<plString>();
  if (shown != m_sCode)
  {
    m_sCode = shown;

    if (m_OnEdited.IsValid())
      m_OnEdited();
  }
}

void plAngelScriptCodeView::BuildDocument()
{
  using namespace plEditorUi;

  m_bRebuild = false;

  plUiPanel* root = Panel();
  root->m_Color = Background();

  plUiCodeArea* area = CodeArea(root, m_sAreaName, s_szCodeBinding, s_szLanguage);
  Fill(area);
  area->m_fBorderWidth = 0;
  area->m_bReadOnly = m_bReadOnly;

  GetSurface()->GetBlackboard().SetEntryValue(s_szCodeBinding, m_sCode);
  SetDocument(Finish(root));
}

//////////////////////////////////////////////////////////////////////////
// Window

plAngelScriptAssetDocumentWindow::plAngelScriptAssetDocumentWindow(plAngelScriptAssetDocument* pDocument)
  : plEngineDocumentWindow(pDocument)
{
  m_pAssetDoc = pDocument;

  SetMenuBarActions("AngelScriptAssetMenuBar");
  SetToolBarActions("AngelScriptAssetToolBar");

  // the script gets the width, the properties sit on its right (the plugin's default arrangement for the type)
  m_pCodeView = AddPanel<plAngelScriptCodeView>({"AngelScriptView", "Script"});
  m_pCodeView->m_OnEdited = plMakeDelegate(&plAngelScriptAssetDocumentWindow::OnCodeEdited, this);
  m_pCodeView->m_OnGutterClicked = plMakeDelegate(&plAngelScriptAssetDocumentWindow::OnGutterClicked, this);

  {
    plStringBuilder sAreaName;
    plConversionUtils::ToString(pDocument->GetGuid(), sAreaName);
    sAreaName.Prepend("Script.area.");
    m_pCodeView->SetAreaName(sAreaName);
  }

  UpdateFileContentDisplay();

  if (plScriptTools* pTools = plScriptTools::GetSingleton())
    m_ToolsSubscription = pTools->m_Events.AddEventHandler(plMakeDelegate(&plAngelScriptAssetDocumentWindow::OnScriptToolsEvent, this));

  AddPropertiesPanel({"AngelScriptAssetDockWidget", "Angel Script Properties", plEditorUiDockRole::Right});
  pDocument->GetSelectionManager()->SetSelection(pDocument->GetObjectManager()->GetRootObject()->GetChildren()[0]);

  m_LastEdit = plTime::MakeZero();

  plAssetCurator::GetSingleton()->m_Events.AddEventHandler(plMakeDelegate(&plAngelScriptAssetDocumentWindow::AssetEventHandler, this));
  m_pAssetDoc->GetObjectManager()->m_PropertyEvents.AddEventHandler(plMakeDelegate(&plAngelScriptAssetDocumentWindow::DocumentObjectEventHandler, this));

  RetrieveScriptInfos();
  UpdateDecorations();
}

plAngelScriptAssetDocumentWindow::~plAngelScriptAssetDocumentWindow()
{
  if (plScriptTools* pTools = plScriptTools::GetSingleton())
    pTools->m_Events.RemoveEventHandler(m_ToolsSubscription);

  // the code view reports edits to this window
  DestroyPanels();

  m_pAssetDoc->GetObjectManager()->m_PropertyEvents.RemoveEventHandler(plMakeDelegate(&plAngelScriptAssetDocumentWindow::DocumentObjectEventHandler, this));
  plAssetCurator::GetSingleton()->m_Events.RemoveEventHandler(plMakeDelegate(&plAngelScriptAssetDocumentWindow::AssetEventHandler, this));
}

void plAngelScriptAssetDocumentWindow::DocumentObjectEventHandler(const plDocumentObjectPropertyEvent& e)
{
  if (e.m_EventType == plDocumentObjectPropertyEvent::Type::PropertySet)
  {
    if (!m_bIgnoreCodeChange && (e.m_sProperty == "Source" || e.m_sProperty == "Code"))
    {
      UpdateFileContentDisplay();
    }
  }
}

void plAngelScriptAssetDocumentWindow::StoreInlineDocState()
{
  const plStringBuilder sContent = m_pCodeView->GetCode();

  if (m_pAssetDoc->GetProperties()->m_CodeMode == plAngelScriptCodeMode::Inline && m_pAssetDoc->GetProperties()->m_sCode != sContent)
  {
    m_bIgnoreCodeChange = true;
    PL_SCOPE_EXIT(m_bIgnoreCodeChange = false);

    plObjectCommandAccessor accessor(m_pAssetDoc->GetCommandHistory());
    accessor.StartTransaction("Edit Code");

    const plDocumentObject* pProps = m_pAssetDoc->GetPropertyObject();
    if (pProps)
    {
      accessor.SetValueByName(pProps, "Code", sContent.GetView()).AssertSuccess();
    }

    accessor.FinishTransaction();
  }
}

void plAngelScriptAssetDocumentWindow::OnCodeEdited()
{
  // Edits are stored at most twice a second, as one undo step each.
  if (m_pAssetDoc->GetProperties()->m_CodeMode == plAngelScriptCodeMode::Inline)
  {
    if (!m_Timers.IsActive(m_uiEditTimer))
    {
      m_uiEditTimer = m_Timers.Start(plTime::MakeFromMilliseconds(100), [this]()
        { OnEditTimer(); }, true);
    }

    m_LastEdit = plTime::Now();
  }
}

void plAngelScriptAssetDocumentWindow::OnEditTimer()
{
  if (m_LastEdit > m_LastSave)
  {
    if (plTime::Now() - m_LastSave > plTime::Seconds(0.5))
    {
      m_LastSave = plTime::Now();
      StoreInlineDocState();
    }
  }
  else
  {
    // The text stopped changing: check it in a scratch module so the markers follow the typing.
    m_Timers.Stop(m_uiEditTimer);
    RequestCheck();
  }
}

void plAngelScriptAssetDocumentWindow::RequestCheck()
{
  m_LastCheck = plTime::Now();
  m_pAssetDoc->SyncInfos();

  plDocumentConfigMsgToEngine msg;
  msg.m_sWhatToDo = "Check";
  GetDocument()->SendMessageToEngine(&msg);
}

plString plAngelScriptAssetDocumentWindow::GetLocationKey(const plAngelScriptAssetDocument* pDocument)
{
  if (pDocument->GetProperties()->m_CodeMode == plAngelScriptCodeMode::Inline)
  {
    plStringBuilder sKey = pDocument->GetDocumentPath();
    plEditorApp::GetSingleton()->MakePathDataDirectoryRelative(sKey);
    return sKey;
  }

  return pDocument->GetProperties()->m_sScriptFile;
}

namespace
{
  plMap<plUuid, plUInt32>& GetPendingReveals()
  {
    static plMap<plUuid, plUInt32> s_Reveals;
    return s_Reveals;
  }

  bool IsSameLocation(plStringView a, plStringView b)
  {
    if (a.IsEmpty() || b.IsEmpty())
      return false;

    plStringBuilder sa = a;
    plStringBuilder sb = b;
    sa.MakeCleanPath();
    sb.MakeCleanPath();
    if (sa.IsEqual_NoCase(sb))
      return true;

    // One side may be absolute and the other data directory relative.
    const plStringBuilder& longer = sa.GetElementCount() > sb.GetElementCount() ? sa : sb;
    const plStringBuilder& shorter = sa.GetElementCount() > sb.GetElementCount() ? sb : sa;
    return longer.EndsWith_NoCase(shorter) && longer.GetElementCount() > shorter.GetElementCount() &&
           longer.GetData()[longer.GetElementCount() - shorter.GetElementCount() - 1] == '/';
  }
} // namespace

void plAngelScriptAssetDocumentWindow::RequestReveal(const plUuid& documentGuid, plUInt32 uiLine)
{
  GetPendingReveals()[documentGuid] = uiLine;

  // An open window shows it now; a window that opens later picks it up in its first update.
  if (plScriptTools* pTools = plScriptTools::GetSingleton())
  {
    plScriptToolsEvent e;
    e.m_Type = plScriptToolsEvent::Type::ProblemsChanged;
    pTools->m_Events.Broadcast(e);
  }
}

void plAngelScriptAssetDocumentWindow::OnScriptToolsEvent(const plScriptToolsEvent& e)
{
  PL_IGNORE_UNUSED(e);
  UpdateDecorations();
}

void plAngelScriptAssetDocumentWindow::OnGutterClicked(plUInt32 uiLine)
{
  if (plScriptTools* pTools = plScriptTools::GetSingleton())
    pTools->ToggleBreakpoint(GetLocationKey(m_pAssetDoc), uiLine);
}

void plAngelScriptAssetDocumentWindow::UpdateDecorations()
{
  plScriptTools* pTools = plScriptTools::GetSingleton();
  if (pTools == nullptr || m_pCodeView == nullptr)
    return;

  const plString sKey = GetLocationKey(m_pAssetDoc);
  plStringBuilder sGuid;
  plConversionUtils::ToString(m_pAssetDoc->GetGuid(), sGuid);

  plUiCodeArea::Decorations decorations;

  // Problems of this script: compile problems are stored under the asset, runtime problems name the file.
  for (const plScriptDiagnostic& diag : pTools->GetProblems())
  {
    if (diag.m_sLanguage != "AngelScript" || diag.m_uiLine == 0)
      continue;

    const bool bThisAsset = diag.m_sAsset == sGuid;
    const bool bThisFile = IsSameLocation(diag.m_sFile, sKey);
    if (!bThisFile && !(bThisAsset && diag.m_sFile.IsEmpty()))
      continue;

    auto& marker = decorations.m_Markers.ExpandAndGetRef();
    marker.m_uiLine = diag.m_uiLine;
    marker.m_uiColumn = diag.m_uiColumn;
    marker.m_uiLength = diag.m_uiEndColumn > diag.m_uiColumn ? diag.m_uiEndColumn - diag.m_uiColumn : 0;
    marker.m_uiSeverity = diag.m_Severity == plScriptDiagnosticSeverity::Error ? 2 : diag.m_Severity == plScriptDiagnosticSeverity::Warning ? 1 : 0;
  }

  for (const plScriptBreakpoint& bp : pTools->GetBreakpoints())
  {
    if (bp.m_bEnabled && IsSameLocation(bp.m_Location.m_sFile, sKey))
      decorations.m_Breakpoints.PushBack(bp.m_Location.m_uiLine);
  }

  // The line the debugger stopped at, in the selected frame.
  if (pTools->IsStopped() && pTools->GetSelectedFrame() < pTools->GetFrames().GetCount())
  {
    const plScriptDebugFrame& frame = pTools->GetFrames()[pTools->GetSelectedFrame()];
    if (IsSameLocation(frame.m_Location.m_sFile, sKey))
    {
      decorations.m_uiCurrentLine = frame.m_Location.m_uiLine;
      decorations.m_uiRevealLine = frame.m_Location.m_uiLine;
    }
  }

  if (auto it = GetPendingReveals().Find(m_pAssetDoc->GetGuid()); it.IsValid())
  {
    m_uiHighlightLine = it.Value();
    decorations.m_uiRevealLine = it.Value();
    GetPendingReveals().Remove(it);
  }

  decorations.m_uiHighlightLine = m_uiHighlightLine;
  m_pCodeView->SetDecorations(decorations);
}

void plAngelScriptAssetDocumentWindow::AssetEventHandler(const plAssetCuratorEvent& e)
{
  if (e.m_AssetGuid == m_pAssetDoc->GetGuid())
  {
    if (e.m_Type == plAssetCuratorEvent::Type::AssetUpdated)
    {
      UpdateFileContentDisplay();
    }
  }
}

void plAngelScriptAssetDocumentWindow::UpdateFileContentDisplay()
{
  if (m_pAssetDoc->GetProperties()->m_CodeMode == plAngelScriptCodeMode::Inline)
  {
    m_pCodeView->SetCode(m_pAssetDoc->GetProperties()->m_sCode, false);
    return;
  }

  const auto& sFile = m_pAssetDoc->GetProperties()->m_sScriptFile;

  if (sFile.IsEmpty())
  {
    m_pCodeView->SetCode("No script file specified.", true);
    return;
  }

  plFileReader file;
  if (file.Open(sFile).Failed())
  {
    plStringBuilder message;
    message.SetFormat("Script file '{}' doesn't exist.", sFile);
    m_pCodeView->SetCode(message, true);
    return;
  }

  plStringBuilder content;
  content.ReadAll(file);

  m_pCodeView->SetCode(content, true);
}

void plAngelScriptAssetDocumentWindow::ProcessMessageEventHandler(const plEditorEngineDocumentMsg* pMsg0)
{
  if (auto pMsg = plDynamicCast<const plSimpleDocumentConfigMsgToEditor*>(pMsg0))
  {
    if (pMsg->m_sWhatToDo == "CheckResult")
    {
      // The live check replaces the asset's compile problems; the next transform replaces them again.
      plHybridArray<plScriptDiagnostic, 8> diagnostics;
      plScriptDiagnostic::ReadAll(pMsg->m_sPayload, diagnostics);

      plHybridArray<plScriptDiagnostic, 8> problems;
      for (const plScriptDiagnostic& diag : diagnostics)
      {
        if (!plScriptDiagnostics::IsEndMarker(diag))
          problems.PushBack(diag);
      }

      if (plScriptTools* pTools = plScriptTools::GetSingleton())
      {
        plStringBuilder sGuid;
        plConversionUtils::ToString(m_pAssetDoc->GetGuid(), sGuid);
        pTools->SetAssetDiagnostics(sGuid, problems);
      }
      return;
    }

    if (pMsg->m_sWhatToDo == "SyncExposedParams_Clear")
    {
      m_ExposedParams.Clear();
      m_Dependencies.Clear();
    }

    if (pMsg->m_sWhatToDo == "SyncExposedParams_Add")
    {
      auto& p = m_ExposedParams.ExpandAndGetRef();
      p.m_sName = pMsg->m_sPayload;
      p.m_DefaultValue = pMsg->m_PayloadValue;
    }

    if (pMsg->m_sWhatToDo == "SyncDependencies_Add")
    {
      m_Dependencies.PushBack(pMsg->m_sPayload);
    }

    if (pMsg->m_sWhatToDo == "SyncExposedParams_Finish")
    {
      plCommandHistory* history = m_pAssetDoc->GetCommandHistory();

      plObjectCommandAccessor accessor(history);

      auto pProps = m_pAssetDoc->GetPropertyObject();

      const plInt32 uiNum = accessor.GetCountByName(pProps, "Parameters");

      bool bAnyChange = (uiNum != m_ExposedParams.GetCount());

      plStringBuilder sDecl;

      for (plInt32 ui = 0; ui < uiNum; ++ui)
      {
        const plDocumentObject* pArgObj = accessor.GetChildObjectByName(pProps, "Parameters", ui);

        plVariant name, expose, def;
        accessor.GetValueByName(pArgObj, "Name", name).AssertSuccess();
        accessor.GetValueByName(pArgObj, "Expose", expose).AssertSuccess();
        accessor.GetValueByName(pArgObj, "DefaultValue", def).AssertSuccess();

        const plString sName = name.ConvertTo<plString>();
        bool bExpose = expose.ConvertTo<bool>();
        bool bFound = false;

        for (auto& ep : m_ExposedParams)
        {
          if (ep.m_sName == sName)
          {
            bFound = true;
            ep.m_bExpose = bExpose;

            if (ep.m_DefaultValue != def)
            {
              bAnyChange = true;
            }

            break;
          }
        }

        if (!bFound)
        {
          bAnyChange = true;
        }
      }


      if (bAnyChange || m_pAssetDoc->GetProperties()->m_Dependencies != m_Dependencies)
      {
        accessor.StartTransaction("Sync Parameters");

        // clear the entire array
        accessor.ClearByName(pProps, "Parameters").AssertSuccess();

        // and fill it again
        for (plUInt32 clip = 0; clip < m_ExposedParams.GetCount(); ++clip)
        {
          plUuid newItemGuid = plUuid::MakeUuid();
          accessor.AddObjectByName(pProps, "Parameters", -1, plGetStaticRTTI<plAngelScriptParameter>(), newItemGuid).AssertSuccess();

          const plDocumentObject* pNewItem = accessor.GetObject(newItemGuid);

          sDecl.SetFormat("{} {} = {}", plAngelScriptUtils::VariantTypeToString(m_ExposedParams[clip].m_DefaultValue.GetType()), m_ExposedParams[clip].m_sName, m_ExposedParams[clip].m_DefaultValue);

          accessor.SetValueByName(pNewItem, "Name", m_ExposedParams[clip].m_sName).AssertSuccess();
          accessor.SetValueByName(pNewItem, "Declaration", sDecl.GetData()).AssertSuccess();
          accessor.SetValueByName(pNewItem, "Expose", m_ExposedParams[clip].m_bExpose).AssertSuccess();
          accessor.SetValueByName(pNewItem, "DefaultValue", m_ExposedParams[clip].m_DefaultValue).AssertSuccess();
        }

        // clear the entire array
        accessor.ClearByName(pProps, "Dependencies").AssertSuccess();

        const plAbstractProperty* pPropDeps = accessor.FindPropertyByName(pProps, "Dependencies");

        // and fill it again
        for (plUInt32 clip = 0; clip < m_Dependencies.GetCount(); ++clip)
        {
          accessor.InsertValue(pProps, pPropDeps, m_Dependencies[clip], -1).AssertSuccess();
        }

        accessor.FinishTransaction();
      }
    }
  }
}

/// Appends the non-empty lines of a script info file to a space-separated word list.
static void ReadSet(plStringView sBasePath, plStringView sFile, plStringBuilder& inout_sWords)
{
  const plStringBuilder fullPath(sBasePath, "/", sFile);

  plFileReader file;
  if (file.Open(fullPath).Failed())
    return;

  plStringBuilder tmp;
  tmp.ReadAll(file);

  plDynamicArray<plStringView> lines;
  tmp.Split(false, lines, "\n", "\r");

  for (plStringView line : lines)
  {
    line.Trim();

    if (!line.IsEmpty())
    {
      inout_sWords.Append(" ", line);
    }
  }
}

void plAngelScriptAssetDocumentWindow::RetrieveScriptInfos()
{
  if (g_bRetrievedScriptInfos)
    return;

  g_bRetrievedScriptInfos = true;

  plStringBuilder sBasePath = plToolsProject::GetSingleton()->GetProjectDataFolder();
  sBasePath.AppendPath("AngelScript");

  plDocumentConfigMsgToEngine msg;
  msg.m_sWhatToDo = "RetrieveScriptInfos";
  msg.m_sValue = sBasePath;
  GetDocument()->SendMessageToEngine(&msg);

  ReadSet(sBasePath, "Types.txt", g_sProjectTypes);
  ReadSet(sBasePath, "Namespaces.txt", g_sProjectTypes);
  ReadSet(sBasePath, "GlobalFunctions.txt", g_sProjectBuiltIns);
  ReadSet(sBasePath, "Methods.txt", g_sProjectBuiltIns);
  ReadSet(sBasePath, "Enums.txt", g_sProjectBuiltIns);
  ReadSet(sBasePath, "Properties.txt", g_sProjectBuiltIns);

  plAngelScriptCodeView::RegisterLanguage();
}
