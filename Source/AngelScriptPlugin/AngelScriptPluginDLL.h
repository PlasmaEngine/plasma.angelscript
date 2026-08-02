#pragma once

// Configure the DLL Import/Export Define
#if PL_ENABLED(PL_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_ANGELSCRIPTPLUGIN_LIB
#    define PL_ANGELSCRIPTPLUGIN_DLL PL_DECL_EXPORT
#  elif defined(BUILDSYSTEM_FOLDING_PLUGIN_IMPORTS)
#    define PL_ANGELSCRIPTPLUGIN_DLL
#  else
#    define PL_ANGELSCRIPTPLUGIN_DLL PL_DECL_IMPORT
#  endif
#else
#  define PL_ANGELSCRIPTPLUGIN_DLL
#endif

#define AS_CHECK(x)                                  \
  if (int res = (x); res < 0)                        \
  {                                                  \
    PL_REPORT_FAILURE("AngelScript error: {}", res); \
  }

enum plAsUserData
{
  ScriptInstancePtr = 0,
  RttiPtr = 1,
  FuncFlags = 2,
};
