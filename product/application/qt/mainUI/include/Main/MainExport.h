#pragma once

#ifdef Main_DLL
#ifdef Main_LIB
#define Main_EXPORT _declspec(dllexport)
#else
#define Main_EXPORT _declspec(dllimport)
#endif
#else
#define Main_EXPORT 
#endif
