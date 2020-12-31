#ifndef _SHREK_EXPORTS_H_INCLUDE_GUARD
#define _SHREK_EXPORTS_H_INCLUDE_GUARD

#if defined(_WIN32) && !defined(shrek_BUILD_CORE)
#pragma comment(lib, "shrek1.lib")
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
    #define shrek_IMPORTED_SYMBOL __declspec(dllimport)
    #define shrek_EXPORTED_SYMBOL __declspec(dllexport)
    #define shrek_LOCAL_SYMBOL
#else

#error platform not supported

#endif

#if defined(shrek_BUILD_CORE)
    #define shrek_API_FUNC(RTYPE) shrek_EXPORTED_SYMBOL RTYPE
#else
    #define shrek_API_FUNC(RTYPE) shrek_IMPORTED_SYMBOL RTYPE
#endif

#endif // _SHREK_EXPORTS_H_INCLUDE_GUARD
