#pragma once

#ifdef UICOMPONENT_DLL
#ifdef UICOMPONENT_LIB
#define UICOMPONENT_EXPORT _declspec(dllexport)
#else
#define UICOMPONENT_EXPORT _declspec(dllimport)
#endif
#else
#define UICOMPONENT_EXPORT 
#endif
