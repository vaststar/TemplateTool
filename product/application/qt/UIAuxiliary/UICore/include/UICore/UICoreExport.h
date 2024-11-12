#pragma once

#ifdef UICore_DLL
#ifdef UICore_LIB
#define UICore_EXPORT _declspec(dllexport)
#else
#define UICore_EXPORT _declspec(dllimport)
#endif
#else
#define UICore_EXPORT 
#endif
