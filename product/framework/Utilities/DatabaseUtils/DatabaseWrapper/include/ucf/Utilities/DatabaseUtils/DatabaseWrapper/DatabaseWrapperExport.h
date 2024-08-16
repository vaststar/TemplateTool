#ifndef DatabaseWrapperExport_h__
#define DatabaseWrapperExport_h__

#ifdef DATABASEWRAPPER_DLL
#ifdef DATABASEWRAPPER_LIB
#define DATABASEWRAPPER_EXPORT _declspec(dllexport)
#else
#define DATABASEWRAPPER_EXPORT _declspec(dllimport)
#endif
#else
#define DATABASEWRAPPER_EXPORT 
#endif

#endif //DatabaseWrapperExport_h__