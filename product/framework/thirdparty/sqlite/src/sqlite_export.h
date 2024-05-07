#pragma once

#ifdef WIN32
#pragma warning(disable:4251)//for template class member with dll-export, it will report warning, so, just disable it
#endif

#ifdef SQLITE_BUILD_DLL
#ifdef SQLITE_BUILD_LIB
#define SQLITE_EXPORT _declspec(dllexport)
#else
#define SQLITE_EXPORT _declspec(dllimport)
#endif
#else
#define SQLITE_EXPORT 
#endif

#define SQLITE_API SQLITE_EXPORT