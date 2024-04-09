#pragma once

#ifdef CLIENTGLOBAL_DLL
#ifdef CLIENTGLOBAL_LIB
#define CLIENTGLOBAL_EXPORT _declspec(dllexport)
#else
#define CLIENTGLOBAL_EXPORT _declspec(dllimport)
#endif
#else
#define CLIENTGLOBAL_EXPORT 
#endif