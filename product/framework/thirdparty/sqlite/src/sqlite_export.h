#pragma once

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4251)//for template class member with dll-export, it will report warning, so, just disable it
#endif

#if defined(_WIN32) || defined(_WIN64)
#  ifdef SQLITE_BUILD_DLL
#    ifdef SQLITE_BUILD_LIB
#      define SQLITE_EXPORT _declspec(dllexport)
#    else
#      define SQLITE_EXPORT _declspec(dllimport)
#    endif
#  else
#    define SQLITE_EXPORT
#  endif
#else
#  ifdef SQLITE_BUILD_DLL
#    define SQLITE_EXPORT __attribute__((visibility("default")))
#  else
#    define SQLITE_EXPORT
#  endif
#endif

#define SQLITE_API SQLITE_EXPORT