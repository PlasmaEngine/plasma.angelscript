#pragma once

#include <AngelScriptPlugin/AngelScriptPluginDLL.h>

#include <AngelScript/include/angelscript.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Strings/StringView.h>
#include <Foundation/Threading/Mutex.h>

class plAsStringFactory : public asIStringFactory
{
public:
  plAsStringFactory();
  ~plAsStringFactory();

  const void* GetStringConstant(const char* szData, asUINT length) override;
  int ReleaseStringConstant(const void* pStr) override;
  int GetRawStringData(const void* pStr, char* szData, asUINT* pLength) const override;

  static plAsStringFactory* GetFactory() { return s_pFactory; }

  const plString& StoreString(const plString& sStr);

private:
  static plAsStringFactory* s_pFactory;
  plMutex m_Mutex;
  plSet<plString> m_Strings;
  plSet<plStringView> m_StringViews;
};
