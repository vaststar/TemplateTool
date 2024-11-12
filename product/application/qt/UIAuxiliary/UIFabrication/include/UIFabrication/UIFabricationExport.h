#pragma once

#ifdef UIFabrication_DLL
#ifdef UIFabrication_LIB
#define UIFabrication_EXPORT _declspec(dllexport)
#else
#define UIFabrication_EXPORT _declspec(dllimport)
#endif
#else
#define UIFabrication_EXPORT 
#endif
