#include <AngelScriptPlugin/AngelScriptPluginPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Runtime/AsDebugBackend.h>
#include <AngelScriptPlugin/Runtime/AsEngineSingleton.h>
#include <AngelScriptPlugin/Runtime/AsInstance.h>
#include <AngelScriptPlugin/Utils/AngelScriptUtils.h>
#include <Core/Scripting/ScriptComponent.h>
#include <Core/World/GameObject.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Utilities/ConversionUtils.h>

namespace
{
  thread_local plDynamicArray<plScriptDiagnostic>* s_pDiagnosticCollector = nullptr;

  constexpr int s_iMaxPrimitiveTypeId = asTYPEID_DOUBLE;

  /// A value as text, without touching types the conversion utilities do not know (templates, script objects).
  plString DescribeValue(asIScriptEngine* pEngine, int iTypeId, void* pAddress, bool& out_bEditable)
  {
    out_bEditable = false;

    if (pAddress == nullptr)
      return "?";

    plStringBuilder sText;
    switch (iTypeId)
    {
      case asTYPEID_BOOL:
        out_bEditable = true;
        return *static_cast<bool*>(pAddress) ? "true" : "false";
      case asTYPEID_INT8:
        out_bEditable = true;
        sText.SetFormat("{}", *static_cast<plInt8*>(pAddress));
        return sText;
      case asTYPEID_INT16:
        out_bEditable = true;
        sText.SetFormat("{}", *static_cast<plInt16*>(pAddress));
        return sText;
      case asTYPEID_INT32:
        out_bEditable = true;
        sText.SetFormat("{}", *static_cast<plInt32*>(pAddress));
        return sText;
      case asTYPEID_INT64:
        out_bEditable = true;
        sText.SetFormat("{}", *static_cast<plInt64*>(pAddress));
        return sText;
      case asTYPEID_UINT8:
        out_bEditable = true;
        sText.SetFormat("{}", *static_cast<plUInt8*>(pAddress));
        return sText;
      case asTYPEID_UINT16:
        out_bEditable = true;
        sText.SetFormat("{}", *static_cast<plUInt16*>(pAddress));
        return sText;
      case asTYPEID_UINT32:
        out_bEditable = true;
        sText.SetFormat("{}", *static_cast<plUInt32*>(pAddress));
        return sText;
      case asTYPEID_UINT64:
        out_bEditable = true;
        sText.SetFormat("{}", *static_cast<plUInt64*>(pAddress));
        return sText;
      case asTYPEID_FLOAT:
        out_bEditable = true;
        sText.SetFormat("{}", *static_cast<float*>(pAddress));
        return sText;
      case asTYPEID_DOUBLE:
        out_bEditable = true;
        sText.SetFormat("{}", *static_cast<double*>(pAddress));
        return sText;
      default:
        break;
    }

    asITypeInfo* pType = pEngine->GetTypeInfoById(iTypeId);
    if (pType == nullptr)
      return "?";

    if ((pType->GetFlags() & asOBJ_ENUM) != 0)
    {
      const plInt32 iValue = *static_cast<plInt32*>(pAddress);
      for (asUINT i = 0; i < pType->GetEnumValueCount(); ++i)
      {
        int iEnumValue = 0;
        const char* szName = pType->GetEnumValueByIndex(i, &iEnumValue);
        if (iEnumValue == iValue)
          return plStringBuilder(pType->GetName(), "::", szName);
      }
      sText.SetFormat("{}", iValue);
      return sText;
    }

    if ((iTypeId & asTYPEID_OBJHANDLE) != 0)
    {
      void* pObject = *static_cast<void**>(pAddress);
      if (pObject == nullptr)
        return "null";

      if ((iTypeId & asTYPEID_SCRIPTOBJECT) != 0)
        return plStringBuilder("{", pType->GetName(), "}");

      if (pType->GetUserData(plAsUserData::RttiPtr) != nullptr && plStringView(pType->GetName()) == "plGameObject")
        return plStringBuilder(static_cast<plGameObject*>(pObject)->GetName(), " (plGameObject)");

      return plStringBuilder(pType->GetName(), "@");
    }

    if ((iTypeId & asTYPEID_SCRIPTOBJECT) != 0)
      return plStringBuilder("{", pType->GetName(), "}");

    // Registered engine value types carry their RTTI; only those convert through a variant.
    if (pType->GetUserData(plAsUserData::RttiPtr) != nullptr && pType->GetSubTypeCount() == 0)
    {
      plVariant value;
      if (plAngelScriptUtils::ReadFromAsTypeAtLocation(pEngine, iTypeId, pAddress, value).Succeeded())
      {
        out_bEditable = value.IsA<plString>() || value.IsA<plHashedString>();
        return value.ConvertTo<plString>();
      }
    }

    return plStringBuilder("{", pType->GetName(), "}");
  }

  plResult WriteValue(asIScriptEngine* pEngine, int iTypeId, void* pAddress, plStringView sValue)
  {
    if (pAddress == nullptr)
      return PL_FAILURE;

    plVariant text = plString(sValue);
    plResult res = PL_FAILURE;

    auto convert = [&](plVariantType::Enum type) -> plVariant
    {
      return text.ConvertTo(type, &res);
    };

    switch (iTypeId)
    {
      case asTYPEID_BOOL:
      {
        const plVariant v = convert(plVariantType::Bool);
        if (res.Succeeded())
          *static_cast<bool*>(pAddress) = v.Get<bool>();
        return res;
      }
      case asTYPEID_INT8:
      case asTYPEID_INT16:
      case asTYPEID_INT32:
      case asTYPEID_INT64:
      {
        const plVariant v = convert(plVariantType::Int64);
        if (res.Failed())
          return res;
        const plInt64 i = v.Get<plInt64>();
        if (iTypeId == asTYPEID_INT8)
          *static_cast<plInt8*>(pAddress) = static_cast<plInt8>(i);
        else if (iTypeId == asTYPEID_INT16)
          *static_cast<plInt16*>(pAddress) = static_cast<plInt16>(i);
        else if (iTypeId == asTYPEID_INT32)
          *static_cast<plInt32*>(pAddress) = static_cast<plInt32>(i);
        else
          *static_cast<plInt64*>(pAddress) = i;
        return PL_SUCCESS;
      }
      case asTYPEID_UINT8:
      case asTYPEID_UINT16:
      case asTYPEID_UINT32:
      case asTYPEID_UINT64:
      {
        const plVariant v = convert(plVariantType::UInt64);
        if (res.Failed())
          return res;
        const plUInt64 u = v.Get<plUInt64>();
        if (iTypeId == asTYPEID_UINT8)
          *static_cast<plUInt8*>(pAddress) = static_cast<plUInt8>(u);
        else if (iTypeId == asTYPEID_UINT16)
          *static_cast<plUInt16*>(pAddress) = static_cast<plUInt16>(u);
        else if (iTypeId == asTYPEID_UINT32)
          *static_cast<plUInt32*>(pAddress) = static_cast<plUInt32>(u);
        else
          *static_cast<plUInt64*>(pAddress) = u;
        return PL_SUCCESS;
      }
      case asTYPEID_FLOAT:
      {
        const plVariant v = convert(plVariantType::Float);
        if (res.Succeeded())
          *static_cast<float*>(pAddress) = v.Get<float>();
        return res;
      }
      case asTYPEID_DOUBLE:
      {
        const plVariant v = convert(plVariantType::Double);
        if (res.Succeeded())
          *static_cast<double*>(pAddress) = v.Get<double>();
        return res;
      }
      default:
        break;
    }

    asITypeInfo* pType = pEngine->GetTypeInfoById(iTypeId);
    if (pType != nullptr && (iTypeId & (asTYPEID_OBJHANDLE | asTYPEID_SCRIPTOBJECT)) == 0 && pType->GetUserData(plAsUserData::RttiPtr) != nullptr &&
        pType->GetSubTypeCount() == 0)
    {
      return plAngelScriptUtils::WriteToAsTypeAtLocation(pEngine, iTypeId, pAddress, text);
    }

    return PL_FAILURE;
  }

  plAngelScriptInstance* GetAsInstance(const plScriptCallFrame& entry)
  {
    if (entry.m_pInstance == nullptr || entry.m_pInstance->GetScriptLanguage() != plAngelScriptDebugBackend::GetLanguageName())
      return nullptr;

    return const_cast<plAngelScriptInstance*>(static_cast<const plAngelScriptInstance*>(entry.m_pInstance));
  }
} // namespace

// Diagnostics.

void plAngelScriptEngineSingleton::SetDiagnosticCollector(plDynamicArray<plScriptDiagnostic>* pCollector)
{
  s_pDiagnosticCollector = pCollector;
}

plDynamicArray<plScriptDiagnostic>* plAngelScriptEngineSingleton::GetDiagnosticCollector()
{
  return s_pDiagnosticCollector;
}

void plAngelScriptEngineSingleton::RegisterSourceMap(const asIScriptModule* pModule, plStringView sProcessedCode)
{
  plUniquePtr<SourceMap> pMap = PL_DEFAULT_NEW(SourceMap);
  pMap->m_sCode = sProcessedCode;

  plHybridArray<plStringView, 256> lines;
  pMap->m_sCode.Split(true, lines, "\n");

  // Each "//#ln <line> "<file>"" marker names the original place of the row after it.
  plUInt16 uiFile = 0;
  plInt32 iLine = 0;
  pMap->m_Files.PushBack(plString());

  for (plStringView line : lines)
  {
    pMap->m_RowFile.PushBack(uiFile);
    pMap->m_RowLine.PushBack(iLine);
    ++iLine;

    plStringView marker = line;
    marker.Trim(" \t\r");
    if (!marker.StartsWith("//#ln"))
      continue;

    marker.TrimWordStart("//#ln ");
    const char* szParsePos = nullptr;
    plInt32 iMarkerLine = 0;
    if (plConversionUtils::StringToInt(marker, iMarkerLine, &szParsePos).Failed() || szParsePos == nullptr)
      continue;

    marker.SetStartPosition(szParsePos);
    marker.Trim(" \"");

    // The markers hold the absolute paths of the machine that compiled the script; breakpoints and problems use
    // data directory relative paths, so the map stores those where the file is in a data directory.
    plStringBuilder sFile = marker;
    plStringBuilder sRelative;
    if (plPathUtils::IsAbsolutePath(sFile) && plFileSystem::ResolvePath(sFile, nullptr, &sRelative).Succeeded() && !sRelative.IsEmpty())
      sFile = sRelative;

    plUInt32 uiIndex = pMap->m_Files.IndexOf(sFile);
    if (uiIndex == plInvalidIndex)
    {
      uiIndex = pMap->m_Files.GetCount();
      pMap->m_Files.PushBack(sFile);
    }

    uiFile = static_cast<plUInt16>(uiIndex);
    iLine = iMarkerLine;
  }

  PL_LOCK(m_SourceMapMutex);
  m_SourceMaps[pModule] = std::move(pMap);
}

void plAngelScriptEngineSingleton::UnregisterSourceMap(const asIScriptModule* pModule)
{
  PL_LOCK(m_SourceMapMutex);
  m_SourceMaps.Remove(pModule);
}

plResult plAngelScriptEngineSingleton::MapLocation(const asIScriptModule* pModule, plInt32 iRow, plStringView& out_sFile, plInt32& out_iLine) const
{
  PL_LOCK(m_SourceMapMutex);

  auto it = m_SourceMaps.Find(pModule);
  if (!it.IsValid())
    return PL_FAILURE;

  const SourceMap& map = *it.Value();
  const plInt32 iIndex = iRow - 1;
  if (iIndex < 0 || iIndex >= static_cast<plInt32>(map.m_RowFile.GetCount()))
    return PL_FAILURE;

  out_sFile = map.m_Files[map.m_RowFile[iIndex]];
  out_iLine = map.m_RowLine[iIndex];
  return out_sFile.IsEmpty() ? PL_FAILURE : PL_SUCCESS;
}

void plAngelScriptEngineSingleton::RegisterFormerNames(const asIScriptModule* pModule, plStringView sCode)
{
  plHashTable<plString, plString> renames;
  plHybridArray<plString, 2> pending; // the @was names of the comment block above the current line

  plHybridArray<plStringView, 256> lines;
  sCode.Split(true, lines, "\n");
  for (plStringView line : lines)
  {
    line.Trim(" \t\r");

    if (line.StartsWith("///"))
    {
      plStringView text = line;
      text.Shrink(3, 0);
      text.Trim(" \t");
      if (text.StartsWith("@was "))
      {
        text.Shrink(5, 0);
        text.Trim(" \t");
        pending.PushBack(text);
      }
      continue;
    }

    if (pending.IsEmpty())
      continue;

    // the declared name: the identifier in front of '=' or ';'
    const char* szEnd = line.FindSubString("=");
    if (szEnd == nullptr)
      szEnd = line.FindSubString(";");

    if (szEnd != nullptr)
    {
      const char* szNameEnd = szEnd;
      while (szNameEnd > line.GetStartPointer() && (*(szNameEnd - 1) == ' ' || *(szNameEnd - 1) == '\t'))
        --szNameEnd;
      const char* szNameStart = szNameEnd;
      while (szNameStart > line.GetStartPointer() && !plStringUtils::IsIdentifierDelimiter_C_Code(*(szNameStart - 1)))
        --szNameStart;

      const plStringView sName(szNameStart, szNameEnd);
      if (!sName.IsEmpty())
      {
        for (const plString& sOld : pending)
          renames[sOld] = sName;
      }
    }

    pending.Clear();
  }

  if (renames.IsEmpty())
    return;

  PL_LOCK(m_SourceMapMutex);
  plUniquePtr<SourceMap>& pMap = m_SourceMaps[pModule];
  if (pMap == nullptr)
    pMap = PL_DEFAULT_NEW(SourceMap);
  pMap->m_Renames = std::move(renames);
}

plStringView plAngelScriptEngineSingleton::FindRenamedField(const asIScriptModule* pModule, plStringView sOldName) const
{
  PL_LOCK(m_SourceMapMutex);

  auto it = m_SourceMaps.Find(pModule);
  if (!it.IsValid())
    return {};

  const plString* pNew = it.Value()->m_Renames.GetValue(sOldName);
  return pNew != nullptr ? pNew->GetView() : plStringView();
}

// Instances.

void plAngelScriptEngineSingleton::AddInstance(plAngelScriptInstance* pInstance)
{
  m_Instances.Insert(pInstance);

  if (m_pDebugBackend != nullptr)
    m_pDebugBackend->UpdateContext(pInstance);
}

void plAngelScriptEngineSingleton::RemoveInstance(plAngelScriptInstance* pInstance)
{
  m_Instances.Remove(pInstance);
}

// Debug backend.

plAngelScriptDebugBackend::plAngelScriptDebugBackend() = default;
plAngelScriptDebugBackend::~plAngelScriptDebugBackend() = default;

void plAngelScriptDebugBackend::OnDebugStateChanged()
{
  // Every line runs the callback while a session is attached, so stepping and pause work in any script.
  const bool bHook = plScriptDebugger::IsAttached();
  if (bHook == m_bHooked)
    return;

  m_bHooked = bHook;

  if (plAngelScriptEngineSingleton* pSingleton = plAngelScriptEngineSingleton::GetSingleton())
  {
    for (plAngelScriptInstance* pInstance : pSingleton->GetInstances())
      UpdateContext(pInstance);
  }
}

void plAngelScriptDebugBackend::UpdateContext(plAngelScriptInstance* pInstance)
{
  asIScriptContext* pContext = pInstance->GetContext();
  if (pContext == nullptr)
    return;

  if (m_bHooked)
    pContext->SetLineCallback(asFUNCTION(&plAngelScriptDebugBackend::LineCallback), this, asCALL_CDECL);
  else
    pContext->ClearLineCallback();
}

plScriptDebugLocation plAngelScriptDebugBackend::GetLocation(asIScriptContext* pContext, plUInt32 uiStackLevel)
{
  plScriptDebugLocation location;

  int iColumn = 0;
  const char* szSection = nullptr;
  const int iRow = pContext->GetLineNumber(uiStackLevel, &iColumn, &szSection);

  location.m_uiLine = iRow > 0 ? static_cast<plUInt32>(iRow) : 0;
  location.m_uiColumn = iColumn > 0 ? static_cast<plUInt32>(iColumn) : 0;
  location.m_sFile = szSection != nullptr ? szSection : "";

  asIScriptFunction* pFunction = pContext->GetFunction(uiStackLevel);
  if (pFunction != nullptr && iRow > 0)
  {
    plStringView sFile;
    plInt32 iLine = 0;
    if (plAngelScriptEngineSingleton::GetSingleton()->MapLocation(pFunction->GetModule(), iRow, sFile, iLine).Succeeded())
    {
      location.m_sFile = sFile;
      location.m_uiLine = static_cast<plUInt32>(plMath::Max(iLine, 0));
    }
  }

  return location;
}

void plAngelScriptDebugBackend::LineCallback(asIScriptContext* pContext, void* pParam)
{
  plAngelScriptDebugBackend* pThis = static_cast<plAngelScriptDebugBackend*>(pParam);

  if (!plScriptDebugger::IsStepping())
  {
    // Cheap test first: only files with breakpoints need a full location.
    int iColumn = 0;
    const char* szSection = nullptr;
    const int iRow = pContext->GetLineNumber(0, &iColumn, &szSection);
    asIScriptFunction* pFunction = pContext->GetFunction(0);

    plStringView sFile;
    plInt32 iLine = 0;
    if (pFunction == nullptr || plAngelScriptEngineSingleton::GetSingleton()->MapLocation(pFunction->GetModule(), iRow, sFile, iLine).Failed())
      return;

    if (!plScriptDebugger::HasBreakpointInFile(sFile))
      return;
  }

  plScriptDebugger::OnLine(pThis, GetLocation(pContext, 0), pContext->GetCallstackSize());
}

void plAngelScriptDebugBackend::CollectFrames(const plScriptCallFrame& entry, plDynamicArray<plScriptDebugFrame>& out_frames)
{
  plAngelScriptInstance* pInstance = GetAsInstance(entry);
  if (pInstance == nullptr || pInstance->GetContext() == nullptr)
    return;

  asIScriptContext* pContext = pInstance->GetContext();
  const plStringView sOwner = pInstance->GetOwnerComponent() != nullptr ? pInstance->GetOwnerComponent()->GetOwner()->GetName() : plStringView();

  for (plUInt32 uiLevel = 0; uiLevel < pContext->GetCallstackSize(); ++uiLevel)
  {
    asIScriptFunction* pFunction = pContext->GetFunction(uiLevel);
    if (pFunction == nullptr)
      continue;

    plScriptDebugFrame& frame = out_frames.ExpandAndGetRef();
    frame.m_sLanguage = GetLanguageName();
    frame.m_sFunction = plAngelScriptUtils::GetNiceFunctionDeclaration(pFunction, true, false);
    frame.m_sInstance = sOwner;
    frame.m_Location = GetLocation(pContext, uiLevel);
    frame.m_bInspectable = true;
  }
}

void plAngelScriptDebugBackend::GetVariables(const plScriptCallFrame& entry, plUInt32 uiLocalFrame, plDynamicArray<plScriptDebugVariable>& out_variables)
{
  plAngelScriptInstance* pInstance = GetAsInstance(entry);
  if (pInstance == nullptr || pInstance->GetContext() == nullptr)
    return;

  asIScriptContext* pContext = pInstance->GetContext();
  asIScriptEngine* pEngine = pContext->GetEngine();
  if (uiLocalFrame >= pContext->GetCallstackSize())
    return;

  // Locals and parameters in scope at the stopped line.
  for (int i = 0; i < pContext->GetVarCount(uiLocalFrame); ++i)
  {
    if (!pContext->IsVarInScope(i, uiLocalFrame))
      continue;

    const char* szName = nullptr;
    int iTypeId = 0;
    pContext->GetVar(i, uiLocalFrame, &szName, &iTypeId);
    if (szName == nullptr || szName[0] == '\0')
      continue;

    plScriptDebugVariable& var = out_variables.ExpandAndGetRef();
    var.m_sName = szName;
    var.m_sType = pEngine->GetTypeDeclaration(iTypeId, true);
    var.m_sValue = DescribeValue(pEngine, iTypeId, pContext->GetAddressOfVar(i, uiLocalFrame), var.m_bEditable);
  }

  // The members of the script object, as "this.Name".
  if (asIScriptObject* pObject = static_cast<asIScriptObject*>(pContext->GetThisPointer(uiLocalFrame)))
  {
    for (asUINT i = 0; i < pObject->GetPropertyCount(); ++i)
    {
      const int iTypeId = pObject->GetPropertyTypeId(i);

      plScriptDebugVariable& var = out_variables.ExpandAndGetRef();
      var.m_sName = plStringBuilder("this.", pObject->GetPropertyName(i));
      var.m_sType = pEngine->GetTypeDeclaration(iTypeId, true);
      var.m_sValue = DescribeValue(pEngine, iTypeId, pObject->GetAddressOfProperty(i), var.m_bEditable);
    }
  }
}

plResult plAngelScriptDebugBackend::SetVariable(const plScriptCallFrame& entry, plUInt32 uiLocalFrame, plStringView sName, plStringView sValue)
{
  plAngelScriptInstance* pInstance = GetAsInstance(entry);
  if (pInstance == nullptr || pInstance->GetContext() == nullptr)
    return PL_FAILURE;

  asIScriptContext* pContext = pInstance->GetContext();
  asIScriptEngine* pEngine = pContext->GetEngine();
  if (uiLocalFrame >= pContext->GetCallstackSize())
    return PL_FAILURE;

  if (sName.StartsWith("this."))
  {
    sName.Shrink(5, 0);
    asIScriptObject* pObject = static_cast<asIScriptObject*>(pContext->GetThisPointer(uiLocalFrame));
    if (pObject == nullptr)
      return PL_FAILURE;

    for (asUINT i = 0; i < pObject->GetPropertyCount(); ++i)
    {
      if (sName == pObject->GetPropertyName(i))
        return WriteValue(pEngine, pObject->GetPropertyTypeId(i), pObject->GetAddressOfProperty(i), sValue);
    }
    return PL_FAILURE;
  }

  for (int i = 0; i < pContext->GetVarCount(uiLocalFrame); ++i)
  {
    const char* szName = nullptr;
    int iTypeId = 0;
    pContext->GetVar(i, uiLocalFrame, &szName, &iTypeId);
    if (szName != nullptr && sName == szName && pContext->IsVarInScope(i, uiLocalFrame))
      return WriteValue(pEngine, iTypeId, pContext->GetAddressOfVar(i, uiLocalFrame), sValue);
  }

  return PL_FAILURE;
}
