#include <AngelScriptPlugin/AngelScriptPluginPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Runtime/AsEngineSingleton.h>
#include <AngelScriptPlugin/Runtime/AsStringFactory.h>
#include <AngelScriptPlugin/Utils/AngelScriptUtils.h>
#include <Core/World/GameObject.h>
#include <Core/World/World.h>

//////////////////////////////////////////////////////////////////////////
// plStringBase
//////////////////////////////////////////////////////////////////////////

template <typename T>
void RegisterStringBase(asIScriptEngine* pEngine, const char* szType)
{
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "bool StartsWith(plStringView) const", asMETHOD(T, StartsWith), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "bool StartsWith_NoCase(plStringView) const", asMETHOD(T, StartsWith_NoCase), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "bool EndsWith(plStringView) const", asMETHOD(T, EndsWith), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "bool EndsWith_NoCase(plStringView) const", asMETHOD(T, EndsWith_NoCase), asCALL_THISCALL));

  // FindSubString
  // FindSubString_NoCase
  // FindLastSubString
  // FindLastSubString_NoCase
  // FindWholeWord
  // FindWholeWord_NoCase

  AS_CHECK(pEngine->RegisterObjectMethod(szType, "int Compare(plStringView) const", asMETHOD(T, Compare), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "int Compare_NoCase(plStringView) const", asMETHOD(T, Compare_NoCase), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "int CompareN(plStringView, uint32) const", asMETHOD(T, CompareN), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "int CompareN_NoCase(plStringView, uint32) const", asMETHOD(T, CompareN_NoCase), asCALL_THISCALL));

  AS_CHECK(pEngine->RegisterObjectMethod(szType, "uint32 GetElementCount() const", asMETHOD(T, GetElementCount), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "bool IsEmpty() const", asMETHOD(T, IsEmpty), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "bool IsEqual(plStringView) const", asMETHOD(T, IsEqual), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "bool IsEqual_NoCase(plStringView) const", asMETHOD(T, IsEqual_NoCase), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "bool IsEqualN(plStringView, uint32) const", asMETHOD(T, IsEqualN), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "bool IsEqualN_NoCase(plStringView, uint32) const", asMETHOD(T, IsEqualN_NoCase), asCALL_THISCALL));

  AS_CHECK(pEngine->RegisterObjectMethod(szType, "bool HasAnyExtension() const", asMETHOD(T, HasAnyExtension), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "bool HasExtension(plStringView) const", asMETHOD(T, HasExtension), asCALL_THISCALL));

  AS_CHECK(pEngine->RegisterObjectMethod(szType, "plStringView GetFileExtension(bool full = false) const", asMETHOD(T, GetFileExtension), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "plStringView GetFileName() const", asMETHOD(T, GetFileName), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "plStringView GetFileNameAndExtension() const", asMETHOD(T, GetFileNameAndExtension), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "plStringView GetFileDirectory() const", asMETHOD(T, GetFileDirectory), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "bool IsAbsolutePath() const", asMETHOD(T, IsAbsolutePath), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "bool IsRelativePath() const", asMETHOD(T, IsRelativePath), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "bool IsRootedPath() const", asMETHOD(T, IsRootedPath), asCALL_THISCALL));
  AS_CHECK(pEngine->RegisterObjectMethod(szType, "bool GetRootedPathRootName() const", asMETHOD(T, GetRootedPathRootName), asCALL_THISCALL));
}

//////////////////////////////////////////////////////////////////////////
// plStringView
//////////////////////////////////////////////////////////////////////////

static int plStringView_opCmp(plStringView* lhs, const plStringView& rhs)
{
  if (*lhs < rhs)
    return -1;
  if (rhs < *lhs)
    return +1;

  return 0;
}

static void plStringView_Construct(void* pMemory)
{
  new (pMemory) plStringView();
}

static void plStringView_ConstructView(void* pMemory, const plStringView rhs)
{
  new (pMemory) plStringView(rhs);
}

static void plStringView_ConstructString(void* pMemory, const plString& rhs)
{
  const plString& str = plAsStringFactory::GetFactory()->StoreString(rhs);

  new (pMemory) plStringView(str);
}

static void plStringView_opAssignString(plStringView* lhs, const plString& rhs)
{
  const plString& str = plAsStringFactory::GetFactory()->StoreString(rhs);

  *lhs = str;
}

static void plStringView_ConstructStringBuilder(void* pMemory, const plStringBuilder& rhs)
{
  const plString& str = plAsStringFactory::GetFactory()->StoreString(rhs);

  new (pMemory) plStringView(str);
}

static void plStringView_opAssignStringBuilder(plStringView* lhs, const plStringBuilder& rhs)
{
  const plString& str = plAsStringFactory::GetFactory()->StoreString(rhs);

  *lhs = str;
}

static void plStringView_ConstructHS(void* pMemory, const plHashedString& rhs)
{
  const plString& str = rhs.GetString();

  new (pMemory) plStringView(str);
}

static void plStringView_opAssignHS(plStringView* lhs, const plHashedString& rhs)
{
  const plString& str = rhs.GetString();

  *lhs = str;
}

static bool plStringView_opEqual(plStringView* lhs, const plStringView& rhs)
{
  return *lhs == rhs;
}

void plAngelScriptEngineSingleton::Register_StringView()
{
  RegisterStringBase<plStringView>(m_pEngine, "plStringView");

  AS_CHECK(m_pEngine->RegisterObjectMethod("plStringView", "void Shrink(plUInt32 uiShrinkCharsFront, plUInt32 uiShrinkCharsBack)", asMETHOD(plStringView, Shrink), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plStringView", "plStringView GetShrunk(plUInt32 uiShrinkCharsFront, plUInt32 uiShrinkCharsBack = 0) const", asMETHOD(plStringView, GetShrunk), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plStringView", "plStringView GetSubString(plUInt32 uiFirstCharacter, plUInt32 uiNumCharacters) const", asMETHOD(plStringView, GetSubString), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plStringView", "void ChopAwayFirstCharacterUtf8()", asMETHOD(plStringView, ChopAwayFirstCharacterUtf8), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plStringView", "void ChopAwayFirstCharacterAscii()", asMETHOD(plStringView, ChopAwayFirstCharacterAscii), asCALL_THISCALL));
  // Trim
  AS_CHECK(m_pEngine->RegisterObjectMethod("plStringView", "bool TrimWordStart(plStringView sWord)", asMETHOD(plStringView, TrimWordStart), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plStringView", "bool TrimWordEnd(plStringView sWord)", asMETHOD(plStringView, TrimWordEnd), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plStringView", "bool opEquals(const plStringView& in) const", asFUNCTIONPR(plStringView_opEqual, (plStringView*, const plStringView&), bool), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plStringView", "int opCmp(const plStringView& in) const", asFUNCTIONPR(plStringView_opCmp, (plStringView*, const plStringView&), int), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plStringView", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(plStringView_Construct), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plStringView", asBEHAVE_CONSTRUCT, "void f(const plStringView)", asFUNCTION(plStringView_ConstructView), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plStringView", asBEHAVE_CONSTRUCT, "void f(const plString& in)", asFUNCTION(plStringView_ConstructString), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plStringView", asBEHAVE_CONSTRUCT, "void f(const plHashedString& in)", asFUNCTION(plStringView_ConstructHS), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plStringView", "void opAssign(const plString& in)", asFUNCTION(plStringView_opAssignString), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plStringView", "void opAssign(const plHashedString& in)", asFUNCTION(plStringView_opAssignHS), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plStringView", asBEHAVE_CONSTRUCT, "void f(const plStringBuilder& in)", asFUNCTION(plStringView_ConstructStringBuilder), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plStringView", "void opAssign(const plStringBuilder& in)", asFUNCTION(plStringView_opAssignStringBuilder), asCALL_CDECL_OBJFIRST));
}


//////////////////////////////////////////////////////////////////////////
// plString
//////////////////////////////////////////////////////////////////////////

static void plString_Construct(void* pMemory)
{
  new (pMemory) plString();
}

static void plString_Destruct(void* pMemory)
{
  plString* p = (plString*)pMemory;
  p->~plString();
}

static void plString_ConstructView(void* pMemory, plStringView rhs)
{
  new (pMemory) plString(rhs);
}

static void plString_ConstructString(void* pMemory, const plString& rhs)
{
  new (pMemory) plString(rhs);
}

static void plString_ConstructStringBuilder(void* pMemory, const plStringBuilder& rhs)
{
  new (pMemory) plString(rhs);
}

static void plString_ConstructHS(void* pMemory, const plHashedString& rhs)
{
  new (pMemory) plString(rhs.GetView());
}

static int plString_opCmp(const plString& lhs, const plString& rhs)
{
  if (lhs < rhs)
    return -1;
  if (rhs < lhs)
    return +1;

  return 0;
}

static void plString_opAssignString(plString* lhs, const plString& rhs)
{
  *lhs = rhs;
}

static void plString_opAssignStringView(plString* lhs, plStringView rhs)
{
  *lhs = rhs;
}

static void plString_opAssignStringBuilder(plString* lhs, const plStringBuilder& rhs)
{
  *lhs = rhs;
}

static void plString_opAssignHS(plString* lhs, const plHashedString& rhs)
{
  *lhs = rhs.GetView();
}

void plAngelScriptEngineSingleton::Register_String()
{
  RegisterStringBase<plString>(m_pEngine, "plString");

  AS_CHECK(m_pEngine->RegisterObjectMethod("plString", "plStringView GetView() const", asMETHOD(plString, GetView), asCALL_THISCALL));
  // AS_CHECK(m_pEngine->RegisterObjectMethod("plString", "uint32 GetCharacterCount() const", asMETHOD(plString, GetCharacterCount), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plString", "int opCmp(const plString& in) const", asFUNCTIONPR(plString_opCmp, (const plString&, const plString&), int), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plString", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(plString_Construct), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plString", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(plString_Destruct), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plString", asBEHAVE_CONSTRUCT, "void f(const plStringView)", asFUNCTION(plString_ConstructView), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plString", "void opAssign(const plStringView)", asFUNCTION(plString_opAssignStringView), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plString", asBEHAVE_CONSTRUCT, "void f(const plString& in)", asFUNCTION(plString_ConstructString), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plString", "void opAssign(const plString& in)", asFUNCTION(plString_opAssignString), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plString", asBEHAVE_CONSTRUCT, "void f(const plStringBuilder& in)", asFUNCTION(plString_ConstructStringBuilder), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plString", "void opAssign(const plStringBuilder& in)", asFUNCTION(plString_opAssignStringBuilder), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plString", asBEHAVE_CONSTRUCT, "void f(const plHashedString& in)", asFUNCTION(plString_ConstructHS), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plString", "void opAssign(const plHashedString& in)", asFUNCTION(plString_opAssignHS), asCALL_CDECL_OBJFIRST));

  // Methods
  {
    AS_CHECK(m_pEngine->RegisterObjectMethod("plString", "void Clear()", asMETHOD(plString, Clear), asCALL_THISCALL));
  }
}

//////////////////////////////////////////////////////////////////////////
// plStringBuilder
//////////////////////////////////////////////////////////////////////////

static void plStringBuilder_Construct(void* pMemory)
{
  new (pMemory) plStringBuilder();
}

static void plStringBuilder_ConstructSV1(void* pMemory, const plStringView sView)
{
  new (pMemory) plStringBuilder(sView);
}

static void plStringBuilder_ConstructSV2(void* pMemory, const plStringView& sV1, const plStringView& sV2)
{
  new (pMemory) plStringBuilder(sV1, sV2);
}

static void plStringBuilder_ConstructSV3(void* pMemory, const plStringView& sV1, const plStringView& sV2, const plStringView& sV3)
{
  new (pMemory) plStringBuilder(sV1, sV2, sV3);
}

static void plStringBuilder_ConstructSV4(void* pMemory, const plStringView& sV1, const plStringView& sV2, const plStringView& sV3, const plStringView& sV4)
{
  new (pMemory) plStringBuilder(sV1, sV2, sV3, sV4);
}

static void plStringBuilder_Destruct(void* pMemory)
{
  plStringBuilder* p = (plStringBuilder*)pMemory;
  p->~plStringBuilder();
}

static void plStringBuilder_Format(plStringBuilder& ref_sStr, asIScriptGeneric* pGen)
{
  const plUInt32 uiNumArgs = (plUInt32)pGen->GetArgCount();
  const plStringView sText = *((plStringView*)pGen->GetArgObject(0));

  plHybridArray<plString, 12> stringStorage;
  plHybridArray<plStringView, 12> stringViews;
  stringStorage.Reserve(pGen->GetArgCount() - 1);

  plVariant res;
  for (plUInt32 uiArg = 1; uiArg < uiNumArgs; ++uiArg)
  {
    auto argTypeId = pGen->GetArgTypeId(uiArg);

    if (plAngelScriptUtils::ReadFromAsTypeAtLocation(pGen->GetEngine(), argTypeId, pGen->GetArgAddress(uiArg), res).Succeeded())
    {
      stringStorage.PushBack(res.ConvertTo<plString>());
      continue;
    }

    const char* typeName = "null";
    if (const asITypeInfo* pInfo = pGen->GetEngine()->GetTypeInfoById(argTypeId))
    {
      typeName = pInfo->GetName();
    }

    plLog::Error("Call to 'plStringBuilder::SetFormat': Argument {} got an unsupported type '{}' ({})", uiArg, typeName, argTypeId);
    break;
  }

  stringViews.Reserve(stringStorage.GetCount());
  for (auto& s : stringStorage)
  {
    stringViews.PushBack(s);
  }

  plFormatString fs(sText);
  fs.BuildFormattedText(ref_sStr, stringViews.GetData(), stringViews.GetCount());
}

static void plStringBuilder_SetFormat(asIScriptGeneric* pGen)
{
  plStringBuilder& sb = *((plStringBuilder*)pGen->GetObject());
  sb.Clear();

  plStringBuilder_Format(sb, pGen);
}

static void plStringBuilder_AppendFormat(asIScriptGeneric* pGen)
{
  plStringBuilder& sb = *((plStringBuilder*)pGen->GetObject());

  plStringBuilder tmp;
  plStringBuilder_Format(tmp, pGen);
  sb.Append(tmp);
}

static void plStringBuilder_PrependFormat(asIScriptGeneric* pGen)
{
  plStringBuilder& sb = *((plStringBuilder*)pGen->GetObject());

  plStringBuilder tmp;
  plStringBuilder_Format(tmp, pGen);
  sb.Prepend(tmp);
}

void plAngelScriptEngineSingleton::Register_StringBuilder()
{
  RegisterStringBase<plStringBuilder>(m_pEngine, "plStringBuilder");

  // Constructors
  {
    AS_CHECK(m_pEngine->RegisterObjectBehaviour("plStringBuilder", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(plStringBuilder_Destruct), asCALL_CDECL_OBJFIRST));

    AS_CHECK(m_pEngine->RegisterObjectBehaviour("plStringBuilder", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(plStringBuilder_Construct), asCALL_CDECL_OBJFIRST));
    AS_CHECK(m_pEngine->RegisterObjectBehaviour("plStringBuilder", asBEHAVE_CONSTRUCT, "void f(const plStringView s1)", asFUNCTION(plStringBuilder_ConstructSV1), asCALL_CDECL_OBJFIRST));
    AS_CHECK(m_pEngine->RegisterObjectBehaviour("plStringBuilder", asBEHAVE_CONSTRUCT, "void f(const plStringView& in, const plStringView& in)", asFUNCTION(plStringBuilder_ConstructSV2), asCALL_CDECL_OBJFIRST));
    AS_CHECK(m_pEngine->RegisterObjectBehaviour("plStringBuilder", asBEHAVE_CONSTRUCT, "void f(const plStringView& in, const plStringView& in, const plStringView& in)", asFUNCTION(plStringBuilder_ConstructSV3), asCALL_CDECL_OBJFIRST));
    AS_CHECK(m_pEngine->RegisterObjectBehaviour("plStringBuilder", asBEHAVE_CONSTRUCT, "void f(const plStringView& in, const plStringView& in, const plStringView& in, const plStringView& in)", asFUNCTION(plStringBuilder_ConstructSV4), asCALL_CDECL_OBJFIRST));
  }

  // Operators
  {
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void opAssign(const plStringBuilder& in rhs)", asMETHODPR(plStringBuilder, operator=, (const plStringBuilder&), void), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void opAssign(plStringView rhs)", asMETHODPR(plStringBuilder, operator=, (plStringView), void), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void opAssign(const plString& in rhs)", asMETHODPR(plStringBuilder, operator=, (const plString&), void), asCALL_THISCALL));
  }

  // Methods
  {
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "plStringView GetView() const", asMETHOD(plStringBuilder, GetView), asCALL_THISCALL));

    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void Clear()", asMETHOD(plStringBuilder, Clear), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "plUInt32 GetCharacterCount() const", asMETHOD(plStringBuilder, GetCharacterCount), asCALL_THISCALL));

    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void ToUpper()", asMETHOD(plStringBuilder, ToUpper), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void ToLower()", asMETHOD(plStringBuilder, ToLower), asCALL_THISCALL));

    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void Set(plStringView sData1)", asMETHODPR(plStringBuilder, Set, (plStringView), void), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void Set(plStringView sData1, plStringView sData2)", asMETHODPR(plStringBuilder, Set, (plStringView, plStringView), void), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void Set(plStringView sData1, plStringView sData2, plStringView sData3)", asMETHODPR(plStringBuilder, Set, (plStringView, plStringView, plStringView), void), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void Set(plStringView sData1, plStringView sData2, plStringView sData3, plStringView sData4)", asMETHODPR(plStringBuilder, Set, (plStringView, plStringView, plStringView, plStringView), void), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void Set(plStringView sData1, plStringView sData2, plStringView sData3, plStringView sData4, plStringView sData5, plStringView sData6 = \"\")", asMETHODPR(plStringBuilder, Set, (plStringView, plStringView, plStringView, plStringView, plStringView, plStringView), void), asCALL_THISCALL));

    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void SetPath(plStringView sData1, plStringView sData2, plStringView sData3 = \"\", plStringView sData4 = \"\")", asMETHOD(plStringBuilder, SetPath), asCALL_THISCALL));

    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void Append(plStringView sData1)", asMETHODPR(plStringBuilder, Append, (plStringView), void), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void Append(plStringView sData1, plStringView sData2)", asMETHODPR(plStringBuilder, Append, (plStringView, plStringView), void), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void Append(plStringView sData1, plStringView sData2, plStringView sData3)", asMETHODPR(plStringBuilder, Append, (plStringView, plStringView, plStringView), void), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void Append(plStringView sData1, plStringView sData2, plStringView sData3, plStringView sData4)", asMETHODPR(plStringBuilder, Append, (plStringView, plStringView, plStringView, plStringView), void), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void Append(plStringView sData1, plStringView sData2, plStringView sData3, plStringView sData4, plStringView sData5, plStringView sData6 = \"\")", asMETHODPR(plStringBuilder, Append, (plStringView, plStringView, plStringView, plStringView, plStringView, plStringView), void), asCALL_THISCALL));

    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void Prepend(plStringView sData1, plStringView sData2 = \"\", plStringView sData3 = \"\", plStringView sData4 = \"\", plStringView sData5 = \"\", plStringView sData6 = \"\")", asMETHODPR(plStringBuilder, Prepend, (plStringView, plStringView, plStringView, plStringView, plStringView, plStringView), void), asCALL_THISCALL));

    // SetFormat
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void SetFormat(plStringView sText, ?&in VarArg1)", asFUNCTION(plStringBuilder_SetFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void SetFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2)", asFUNCTION(plStringBuilder_SetFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void SetFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3)", asFUNCTION(plStringBuilder_SetFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void SetFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3, ?&in VarArg4)", asFUNCTION(plStringBuilder_SetFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void SetFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3, ?&in VarArg4, ?&in VarArg5)", asFUNCTION(plStringBuilder_SetFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void SetFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3, ?&in VarArg4, ?&in VarArg5, ?&in VarArg6)", asFUNCTION(plStringBuilder_SetFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void SetFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3, ?&in VarArg4, ?&in VarArg5, ?&in VarArg6, ?&in VarArg7)", asFUNCTION(plStringBuilder_SetFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void SetFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3, ?&in VarArg4, ?&in VarArg5, ?&in VarArg6, ?&in VarArg7, ?&in VarArg8)", asFUNCTION(plStringBuilder_SetFormat), asCALL_GENERIC));

    // AppendFormat
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void AppendFormat(plStringView sText, ?&in VarArg1)", asFUNCTION(plStringBuilder_AppendFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void AppendFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2)", asFUNCTION(plStringBuilder_AppendFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void AppendFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3)", asFUNCTION(plStringBuilder_AppendFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void AppendFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3, ?&in VarArg4)", asFUNCTION(plStringBuilder_AppendFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void AppendFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3, ?&in VarArg4, ?&in VarArg5)", asFUNCTION(plStringBuilder_AppendFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void AppendFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3, ?&in VarArg4, ?&in VarArg5, ?&in VarArg6)", asFUNCTION(plStringBuilder_AppendFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void AppendFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3, ?&in VarArg4, ?&in VarArg5, ?&in VarArg6, ?&in VarArg7)", asFUNCTION(plStringBuilder_AppendFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void AppendFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3, ?&in VarArg4, ?&in VarArg5, ?&in VarArg6, ?&in VarArg7, ?&in VarArg8)", asFUNCTION(plStringBuilder_AppendFormat), asCALL_GENERIC));

    // PrependFormat
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void PrependFormat(plStringView sText, ?&in VarArg1)", asFUNCTION(plStringBuilder_PrependFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void PrependFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2)", asFUNCTION(plStringBuilder_PrependFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void PrependFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3)", asFUNCTION(plStringBuilder_PrependFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void PrependFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3, ?&in VarArg4)", asFUNCTION(plStringBuilder_PrependFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void PrependFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3, ?&in VarArg4, ?&in VarArg5)", asFUNCTION(plStringBuilder_PrependFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void PrependFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3, ?&in VarArg4, ?&in VarArg5, ?&in VarArg6)", asFUNCTION(plStringBuilder_PrependFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void PrependFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3, ?&in VarArg4, ?&in VarArg5, ?&in VarArg6, ?&in VarArg7)", asFUNCTION(plStringBuilder_PrependFormat), asCALL_GENERIC));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void PrependFormat(plStringView sText, ?&in VarArg1, ?&in VarArg2, ?&in VarArg3, ?&in VarArg4, ?&in VarArg5, ?&in VarArg6, ?&in VarArg7, ?&in VarArg8)", asFUNCTION(plStringBuilder_PrependFormat), asCALL_GENERIC));

    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void Shrink(plUInt32 uiShrinkCharsFront, plUInt32 uiShrinkCharsBack)", asMETHOD(plStringBuilder, Shrink), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void Reserve(plUInt32 uiNumElements)", asMETHOD(plStringBuilder, Reserve), asCALL_THISCALL));

    // TODO AngelScript: plStringBuilder::ReplaceFirst
    // TODO AngelScript: plStringBuilder::ReplaceLast

    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "plUInt32 ReplaceAll(plStringView sSearchFor, plStringView sReplacement)", asMETHOD(plStringBuilder, ReplaceAll), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "plUInt32 ReplaceAll_NoCase(plStringView sSearchFor, plStringView sReplacement)", asMETHOD(plStringBuilder, ReplaceAll_NoCase), asCALL_THISCALL));

    // TODO AngelScript: plStringBuilder::ReplaceWholeWord
    // TODO AngelScript: plStringBuilder::ReplaceWholeWordAll

    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void MakeCleanPath()", asMETHOD(plStringBuilder, MakeCleanPath), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void PathParentDirectory(plUInt32 uiLevelsUp = 1)", asMETHOD(plStringBuilder, PathParentDirectory), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void AppendPath(plStringView sPath1, plStringView sPath2 = \"\", plStringView sPath3 = \"\", plStringView sPath4 = \"\")", asMETHOD(plStringBuilder, AppendPath), asCALL_THISCALL));

    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void AppendWithSeparator(plStringView sSeparator, plStringView sData1, plStringView sData2 = \"\", plStringView sData3 = \"\", plStringView sData4 = \"\", plStringView sData5 = \"\", plStringView sData6 = \"\")", asMETHOD(plStringBuilder, AppendWithSeparator), asCALL_THISCALL));

    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void ChangeFileName(plStringView sNewFileName)", asMETHOD(plStringBuilder, ChangeFileName), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void ChangeFileNameAndExtension(plStringView sNewFileNameWithExtension)", asMETHOD(plStringBuilder, ChangeFileNameAndExtension), asCALL_THISCALL));

    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void ChangeFileExtension(plStringView sNewExtension, bool bFullExtension = false)", asMETHOD(plStringBuilder, ChangeFileExtension), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "void RemoveFileExtension(bool bFullExtension = false)", asMETHOD(plStringBuilder, RemoveFileExtension), asCALL_THISCALL));

    // TODO AngelScript: plStringBuilder::MakeRelativeTo

    // bool IsPathBelowFolder(const char* szPathToFolder)
    // void Trim(const char* szTrimChars = " \f\n\r\t\v")
    // TrimLeft, TrimRight

    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "bool TrimWordStart(plStringView sWord)", asMETHOD(plStringBuilder, TrimWordStart), asCALL_THISCALL));
    AS_CHECK(m_pEngine->RegisterObjectMethod("plStringBuilder", "bool TrimWordEnd(plStringView sWord)", asMETHOD(plStringBuilder, TrimWordEnd), asCALL_THISCALL));
  }
}

//////////////////////////////////////////////////////////////////////////
// plTempHashedString
//////////////////////////////////////////////////////////////////////////

static void plTempHashedString_Construct(void* pMemory)
{
  new (pMemory) plTempHashedString();
}

static void plTempHashedString_ConstructView(void* pMemory, plStringView sView)
{
  new (pMemory) plTempHashedString(sView);
}

static void plTempHashedString_ConstructTempHashed(void* pMemory, const plTempHashedString& sString)
{
  new (pMemory) plTempHashedString(sString);
}

static void plTempHashedString_ConstructHS(void* pMemory, const plHashedString& sString)
{
  new (pMemory) plTempHashedString(sString);
}

static void plTempHashedString_AssignStringView(plTempHashedString* pStr, plStringView sView)
{
  *pStr = sView;
}

static void plTempHashedString_AssignHS(plTempHashedString* pStr, const plHashedString& sString)
{
  *pStr = sString;
}

void plAngelScriptEngineSingleton::Register_TempHashedString()
{
  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plTempHashedString", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(plTempHashedString_Construct), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plTempHashedString", asBEHAVE_CONSTRUCT, "void f(const plTempHashedString& in)", asFUNCTION(plTempHashedString_ConstructTempHashed), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plTempHashedString", asBEHAVE_CONSTRUCT, "void f(const plStringView)", asFUNCTION(plTempHashedString_ConstructView), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plTempHashedString", asBEHAVE_CONSTRUCT, "void f(const plHashedString& in)", asFUNCTION(plTempHashedString_ConstructHS), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTempHashedString", "void opAssign(plStringView)", asFUNCTION(plTempHashedString_AssignStringView), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTempHashedString", "void opAssign(const plHashedString& in)", asFUNCTION(plTempHashedString_AssignHS), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTempHashedString", "bool opEquals(plTempHashedString) const", asMETHODPR(plTempHashedString, operator==, (const plTempHashedString&) const, bool), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plTempHashedString", "bool IsEmpty() const", asMETHOD(plTempHashedString, IsEmpty), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plTempHashedString", "void Clear()", asMETHOD(plTempHashedString, Clear), asCALL_THISCALL));
}

//////////////////////////////////////////////////////////////////////////
// plHashedString
//////////////////////////////////////////////////////////////////////////

static void plHashedString_Construct(void* pMemory)
{
  new (pMemory) plHashedString();
}

static void plHashedString_ConstructView(void* pMemory, plStringView sView)
{
  plHashedString* obj = new (pMemory) plHashedString();
  obj->Assign(sView);
}

static void plHashedString_ConstructHS(void* pMemory, const plHashedString& sString)
{
  new (pMemory) plHashedString(sString);
}

static void plHashedString_AssignStringView(plHashedString* pStr, const plStringView sView)
{
  pStr->Assign(sView);
}

static bool plHashedString_EqualsStringView(plHashedString* pStr, const plStringView sView)
{
  return *pStr == sView;
}

void plAngelScriptEngineSingleton::Register_HashedString()
{
  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plHashedString", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(plHashedString_Construct), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plHashedString", asBEHAVE_CONSTRUCT, "void f(const plStringView)", asFUNCTION(plHashedString_ConstructView), asCALL_CDECL_OBJFIRST));
  AS_CHECK(m_pEngine->RegisterObjectBehaviour("plHashedString", asBEHAVE_CONSTRUCT, "void f(const plHashedString& in)", asFUNCTION(plHashedString_ConstructHS), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plHashedString", "void opAssign(const plStringView)", asFUNCTION(plHashedString_AssignStringView), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plHashedString", "bool IsEmpty() const", asMETHOD(plHashedString, IsEmpty), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plHashedString", "void Clear()", asMETHOD(plHashedString, Clear), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plHashedString", "void Assign(const plStringView)", asMETHODPR(plHashedString, Assign, (plStringView), void), asCALL_THISCALL));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plHashedString", "bool opEquals(const plHashedString& in) const", asMETHODPR(plHashedString, operator==, (const plHashedString&) const, bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plHashedString", "bool opEquals(const plTempHashedString& in) const", asMETHODPR(plHashedString, operator==, (const plTempHashedString&) const, bool), asCALL_THISCALL));
  AS_CHECK(m_pEngine->RegisterObjectMethod("plHashedString", "bool opEquals(const plStringView) const", asFUNCTION(plHashedString_EqualsStringView), asCALL_CDECL_OBJFIRST));

  AS_CHECK(m_pEngine->RegisterObjectMethod("plHashedString", "plStringView GetView() const", asMETHOD(plHashedString, GetView), asCALL_THISCALL));
}
