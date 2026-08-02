#include <AngelScriptPlugin/AngelScriptPluginPCH.h>

#include <AngelScriptPlugin/Runtime/AsStringFactory.h>
#include <Foundation/Strings/HashedString.h>

plAsStringFactory* plAsStringFactory::s_pFactory = nullptr;

plAsStringFactory::plAsStringFactory()
{
  s_pFactory = this;
}

plAsStringFactory::~plAsStringFactory()
{
  s_pFactory = nullptr;
}

const void* plAsStringFactory::GetStringConstant(const char* szData, asUINT length)
{
  const plString str(plStringView(szData, length));

  // we need to give out a pointer to a StringView that doesn't vanish
  PL_LOCK(m_Mutex);
  auto itStr = m_Strings.Insert(str);
  auto itView = m_StringViews.Insert(itStr.Key().GetView());
  const plStringView& view = itView.Key();

  return &view;
}

int plAsStringFactory::ReleaseStringConstant(const void* pStr)
{
  // we don't clean up the strings
  return 0;
}

int plAsStringFactory::GetRawStringData(const void* pStr, char* szData, asUINT* pLength) const
{
  const plStringView* pView = (const plStringView*)pStr;

  *pLength = pView->GetElementCount();

  if (szData)
  {
    plStringUtils::Copy(szData, *pLength + 1, pView->GetStartPointer());
  }

  return 0;
}

const plString& plAsStringFactory::StoreString(const plString& sStr)
{
  PL_LOCK(m_Mutex);
  auto itStr = m_Strings.Insert(sStr);

  return itStr.Key();
}
