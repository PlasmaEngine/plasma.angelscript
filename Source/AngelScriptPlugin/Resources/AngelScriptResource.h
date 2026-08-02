#pragma once

#include <AngelScriptPlugin/AngelScriptPluginDLL.h>
#include <Core/ResourceManager/ResourceHandle.h>
#include <Core/Scripting/ScriptClassResource.h>

class asIScriptModule;
class asITypeInfo;

using plAngelScriptResourceHandle = plTypedResourceHandle<class plAngelScriptResource>;

class PL_ANGELSCRIPTPLUGIN_DLL plAngelScriptResource : public plScriptClassResource
{
  PL_ADD_DYNAMIC_REFLECTION(plAngelScriptResource, plScriptClassResource);
  PL_RESOURCE_DECLARE_COMMON_CODE(plAngelScriptResource);

public:
  plAngelScriptResource();
  ~plAngelScriptResource();

  plStringView GetScriptContent() const { return m_sScriptContent; }

private:
  virtual plResourceLoadDesc UnloadData(Unload WhatToUnload) override;
  virtual plResourceLoadDesc UpdateContent(plStreamReader* pStream) override;
  virtual void UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage) override;

  virtual plUniquePtr<plScriptInstance> Instantiate(plReflectedClass& inout_owner, plWorld* pWorld) const override;

  void FindMessageHandlers(const asITypeInfo* pClassType, plScriptRTTI::MessageHandlerList& inout_Handlers);

  plString m_sClassName;
  plString m_sScriptContent;
  asIScriptModule* m_pModule = nullptr;
};
