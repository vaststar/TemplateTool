#pragma once

#ifdef APPRUNNER_DLL
#ifdef APPRUNNER_LIB
#define APPRUNNER_EXPORT _declspec(dllexport)
#else
#define APPRUNNER_EXPORT _declspec(dllimport)
#endif
#else
#define APPRUNNER_EXPORT 
#endif