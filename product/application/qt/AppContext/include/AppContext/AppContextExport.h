#pragma once

#ifdef AppContext_DLL
#ifdef AppContext_LIB
#define AppContext_EXPORT _declspec(dllexport)
#else
#define AppContext_EXPORT _declspec(dllimport)
#endif
#else
#define AppContext_EXPORT 
#endif
