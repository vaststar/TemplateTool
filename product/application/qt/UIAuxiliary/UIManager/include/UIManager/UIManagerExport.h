#pragma once

#ifdef UIManager_DLL
#ifdef UIManager_LIB
#define UIManager_EXPORT _declspec(dllexport)
#else
#define UIManager_EXPORT _declspec(dllimport)
#endif
#else
#define UIManager_EXPORT 
#endif
