#include <AngelScriptPlugin/AngelScriptPluginPCH.h>

#include <AngelScript/include/angelscript.h>
#include <AngelScriptPlugin/Resources/AngelScriptResource.h>
#include <AngelScriptPlugin/Runtime/AsEngineSingleton.h>
#include <AngelScriptPlugin/Runtime/AsFunctionDispatch.h>
#include <AngelScriptPlugin/Runtime/AsInstance.h>
#include <AngelScriptPlugin/Utils/AngelScriptUtils.h>
#include <Core/Scripting/ScriptAttributes.h>
#include <Core/World/Component.h>
#include <Foundation/Communication/Message.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/ChunkStream.h>
#include <Foundation/IO/CompressedStreamZstd.h>
#include <Foundation/IO/StringDeduplicationContext.h>
#include <Foundation/Types/ScopeExit.h>
#include <Foundation/Utilities/AssetFileHeader.h>

// clang-format off
PL_BEGIN_DYNAMIC_REFLECTED_TYPE(plAngelScriptResource, 1, plRTTIDefaultAllocator<plAngelScriptResource>)
PL_END_DYNAMIC_REFLECTED_TYPE;
PL_RESOURCE_IMPLEMENT_COMMON_CODE(plAngelScriptResource);

PL_BEGIN_SUBSYSTEM_DECLARATION(AngelScript, AngelScriptResource)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "ResourceManager" 
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP 
  {
    plResourceManager::RegisterResourceForAssetType("AngelScriptClass", plGetStaticRTTI<plAngelScriptResource>());
    // The editor's document type; its reload requests after a transform use this name.
    plResourceManager::RegisterResourceForAssetType("AngelScript", plGetStaticRTTI<plAngelScriptResource>());
    plResourceManager::RegisterResourceOverrideType(plGetStaticRTTI<plAngelScriptResource>(), [](const plStringBuilder& sResourceID) -> bool  {
        return sResourceID.HasExtension(".plBinAngelScript");
      });
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    plResourceManager::UnregisterResourceOverrideType(plGetStaticRTTI<plAngelScriptResource>());
  }

PL_END_SUBSYSTEM_DECLARATION;
// clang-format on

plAngelScriptResource::plAngelScriptResource() = default;
plAngelScriptResource::~plAngelScriptResource() = default;

plResourceLoadDesc plAngelScriptResource::UnloadData(Unload WhatToUnload)
{
  DeleteScriptType();
  DeleteAllScriptCoroutineTypes();

  if (m_pModule)
  {
    // can't do this here, because other worlds may still use the current state
    // need to track somehow where modules are still in use
    // m_pModule->Discard();
    m_pModule = nullptr;
  }

  plResourceLoadDesc ld;
  ld.m_State = plResourceState::Unloaded;
  ld.m_uiQualityLevelsDiscardable = 0;
  ld.m_uiQualityLevelsLoadable = 0;

  return ld;
}

plResourceLoadDesc plAngelScriptResource::UpdateContent(plStreamReader* pStream)
{
  plResourceLoadDesc ld;
  ld.m_uiQualityLevelsDiscardable = 0;
  ld.m_uiQualityLevelsLoadable = 0;
  ld.m_State = plResourceState::LoadedResourceMissing;

  if (pStream == nullptr)
  {
    return ld;
  }

  // the standard file reader writes the absolute file path into the stream
  plString sAbsFilePath;
  (*pStream) >> sAbsFilePath;

  // skip the asset file header at the start of the file
  plAssetFileHeader AssetHash;
  AssetHash.Read(*pStream).IgnoreResult();

  plUInt8 uiVersion = 1;
  (*pStream) >> uiVersion;

  plUInt8 uiCompressionMode = 0;

  if (uiVersion >= 4)
  {
    (*pStream) >> uiCompressionMode;
  }

  plStreamReader* pDeCompressor = pStream;

#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
  plCompressedStreamReaderZstd decompressorZstd;
#endif

  switch (uiCompressionMode)
  {
    case 0:
      break;

    case 1:
#ifdef BUILDSYSTEM_ENABLE_ZSTD_SUPPORT
      decompressorZstd.SetInputStream(pStream);
      pDeCompressor = &decompressorZstd;
      break;
#else
      plLog::Error("AngelScript is compressed with zstandard, but support for this compressor is not compiled in.");
      ld.m_State = plResourceState::LoadedResourceMissing;
      return ld;
#endif

    default:
      plLog::Error("AngelScript is compressed with an unknown algorithm.");
      ld.m_State = plResourceState::LoadedResourceMissing;
      return ld;
  }

  plStreamReader& stream = *pDeCompressor;
  stream >> m_sClassName;

  plStringBuilder sModuleID;
  sModuleID.SetFormat("{}-{}", GetResourceID(), GetCurrentResourceChangeCounter());

  auto pAs = plAngelScriptEngineSingleton::GetSingleton();

  // Problems while loading (a script that compiled in the editor but not here) are reported like runtime problems.
  plHybridArray<plScriptDiagnostic, 4> loadDiagnostics;
  plAngelScriptEngineSingleton::SetDiagnosticCollector(&loadDiagnostics);
  PL_SCOPE_EXIT(plAngelScriptEngineSingleton::SetDiagnosticCollector(nullptr));

  plStringBuilder sProcessedCode;

  if (uiVersion == 1)
  {
    stream >> m_sScriptContent;
    m_pModule = pAs->CompileModule(sModuleID, m_sClassName, "main", m_sScriptContent, &sProcessedCode, nullptr);
  }
  else
  {
    plHybridArray<plUInt8, 1024 * 8> bytecode;
    stream.ReadArray(bytecode).AssertSuccess();

    if (uiVersion >= 3)
    {
      stream >> m_sScriptContent;
      m_pModule = pAs->CompileModule(sModuleID, m_sClassName, "main", m_sScriptContent, &sProcessedCode, nullptr);
    }
    else
    {
      m_pModule = plAngelScriptUtils::LoadFromByteCode(pAs->GetEngine(), sModuleID, bytecode);
    }
  }

  if (m_pModule == nullptr)
  {
    for (plScriptDiagnostic& diag : loadDiagnostics)
    {
      diag.m_Origin = plScriptDiagnosticOrigin::Reload;
      diag.m_sAsset = GetResourceID();
      plScriptDiagnostics::Report(diag);
    }
    return ld;
  }

  if (!sProcessedCode.IsEmpty())
  {
    pAs->RegisterSourceMap(m_pModule, sProcessedCode);
  }

  // values saved under a field's old name still reach it (`/// @was OldName`)
  pAs->RegisterFormerNames(m_pModule, m_sScriptContent);

  const asITypeInfo* pClassType = m_pModule->GetTypeInfoByDecl(m_sClassName);

  plScriptRTTI::FunctionList functions;
  plScriptRTTI::MessageHandlerList messageHandlers;

  const plRTTI* pBaseType = plGetStaticRTTI<plComponent>();

  plStringBuilder sFunctionName;

  plHybridArray<plString, 16> funcNames;

  for (auto pCompFunc : pBaseType->GetFunctions())
  {
    const plScriptBaseClassFunctionAttribute* pAttr = pCompFunc->GetAttributeByType<plScriptBaseClassFunctionAttribute>();
    if (pAttr == nullptr)
      continue;

    sFunctionName = pCompFunc->GetPropertyName();
    sFunctionName.TrimWordStart("Reflection_");

    funcNames.PushBack(sFunctionName);

    if (auto pFunc = pClassType->GetMethodByName(sFunctionName))
    {
      plUniquePtr<plAngelScriptFunctionProperty> pFunctionProperty = PL_SCRIPT_NEW(plAngelScriptFunctionProperty, sFunctionName, pFunc);
      functions.PushBack(std::move(pFunctionProperty));
    }
  }

  FindMessageHandlers(pClassType, messageHandlers);

  if (functions.IsEmpty() && messageHandlers.IsEmpty())
  {
    plLog::Error("AngelScript code doesn't contain any callable function or message handlers.");
    plLog::Info("Candidates are:");

    for (const auto& s : funcNames)
    {
      plLog::Info("  {}", s);
    }

    return ld;
  }

  CreateScriptType(GetResourceID(), pBaseType, std::move(functions), std::move(messageHandlers));

  SetReloadInfo("AngelScript", "A-RECOMPILE", "AngelScript compiled a new module. Instances are re-created; members that still exist keep their values.");

  ld.m_State = plResourceState::Loaded;
  return ld;
}

void plAngelScriptResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = (plUInt32)sizeof(plAngelScriptResource);
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
}

plUniquePtr<plScriptInstance> plAngelScriptResource::Instantiate(plReflectedClass& inout_owner, plWorld* pWorld) const
{
  return PL_SCRIPT_NEW(plAngelScriptInstance, inout_owner, pWorld, m_pModule, m_sClassName);
}

void plAngelScriptResource::FindMessageHandlers(const asITypeInfo* pClassType, plScriptRTTI::MessageHandlerList& inout_Handlers)
{
  auto pAs = plAngelScriptEngineSingleton::GetSingleton();

  plStringBuilder sArgType;

  plUniquePtr<plAngelScriptCustomAsMessageHandler> pAsMsgHandler;

  for (plUInt32 i = 0; i < pClassType->GetMethodCount(); ++i)
  {
    asIScriptFunction* pFunc = pClassType->GetMethodByIndex(i, false);

    if (!plStringUtils::StartsWith(pFunc->GetName(), "OnMsg"))
      continue;

    // only allow void functions
    if (pFunc->GetReturnTypeId() != asTYPEID_VOID)
    {
      plLog::Error("Malformed message handler '{}': Return type must be 'void'.", pFunc->GetName());
      continue;
    }

    // that take exactly one argument
    if (pFunc->GetParamCount() != 1)
    {
      plLog::Error("Malformed message handler '{}': Must take exactly one argument.", pFunc->GetName());
      continue;
    }

    int iArgTypeId;
    pFunc->GetParam(0, &iArgTypeId);

    if (iArgTypeId & asTYPEID_APPOBJECT)
    {
      const plRTTI* pArgType = plAngelScriptUtils::MapToRTTI(iArgTypeId, pAs->GetEngine());

      if (pArgType == nullptr)
      {
        plLog::Error("Malformed message handler '{}': Argument has unknown type.", pFunc->GetName());
        continue;
      }

      // has to be a type derived from plMessage
      if (!pArgType->IsDerivedFrom<plMessage>())
      {
        plLog::Error("Malformed message handler '{}': Argument type has to derive from plMessage or plAngelScriptMessage.", pFunc->GetName());
        continue;
      }

      plScriptMessageDesc desc;
      desc.m_pType = pArgType;

      plUniquePtr<plAngelScriptMessageHandler> pFunctionProperty = PL_SCRIPT_NEW(plAngelScriptMessageHandler, desc, pFunc);
      inout_Handlers.PushBack(std::move(pFunctionProperty));
    }

    if (iArgTypeId & asTYPEID_SCRIPTOBJECT)
    {
      if (auto pArgType = pAs->GetEngine()->GetTypeInfoById(iArgTypeId))
      {
        if (pArgType->GetInterfaceCount() != 1 || !plStringUtils::IsEqual(pArgType->GetInterface(0)->GetName(), "plAngelScriptMessage"))
        {
          plLog::Error("Malformed message handler '{}': Argument type has to derive from plMessage or plAngelScriptMessage.", pFunc->GetName());
          continue;
        }

        if (!pAsMsgHandler)
        {
          plScriptMessageDesc desc;
          desc.m_pType = plGetStaticRTTI<plMsgDeliverAngelScriptMsg>();

          pAsMsgHandler = PL_SCRIPT_NEW(plAngelScriptCustomAsMessageHandler, desc);
        }

        pAsMsgHandler->AddReceiver(pFunc, pArgType->GetName());
      }
    }
  }

  if (pAsMsgHandler)
  {
    inout_Handlers.PushBack(std::move(pAsMsgHandler));
  }
}
