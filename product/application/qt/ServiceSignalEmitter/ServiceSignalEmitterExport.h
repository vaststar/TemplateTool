#pragma once

#ifdef ServiceSignalEmitter_DLL
#ifdef ServiceSignalEmitter_LIB
#define ServiceSignalEmitter_EXPORT _declspec(dllexport)
#else
#define ServiceSignalEmitter_EXPORT _declspec(dllimport)
#endif
#else
#define ServiceSignalEmitter_EXPORT 
#endif
