#ifndef UtilitiesExportExport_h__
#define UtilitiesExportExport_h__

#ifdef Utilities_DLL
#ifdef Utilities_LIB
#define Utilities_EXPORT _declspec(dllexport)
#else
#define Utilities_EXPORT _declspec(dllimport)
#endif
#else
#define Utilities_EXPORT 
#endif

#endif //UtilitiesExportExport_h__