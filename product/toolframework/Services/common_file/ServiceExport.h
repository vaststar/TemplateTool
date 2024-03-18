#ifndef ServiceExportExport_h__
#define ServiceExportExport_h__

#ifdef SERVICE_DLL
#ifdef SERVICE_LIB
#define SERVICE_EXPORT _declspec(dllexport)
#else
#define SERVICE_EXPORT _declspec(dllimport)
#endif
#else
#define SERVICE_EXPORT 
#endif

#endif //ServiceExportExport_h__