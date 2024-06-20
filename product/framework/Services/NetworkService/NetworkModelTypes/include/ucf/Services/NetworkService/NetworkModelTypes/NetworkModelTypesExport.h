#ifndef NetworkModelTypesExport_h__
#define NetworkModelTypesExport_h__

#ifdef NETWORKTYPE_DLL
#ifdef NETWORKTYPE_LIB
#define NETWORKTYPE_EXPORT _declspec(dllexport)
#else
#define NETWORKTYPE_EXPORT _declspec(dllimport)
#endif
#else
#define NETWORKTYPE_EXPORT 
#endif

#endif //NetworkModelTypesExport_h__