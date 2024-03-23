#ifndef ServiceExportExport_h__
#define ServiceExportExport_h__

#ifdef WIN32
#pragma warning(disable:4251)//for template class member with dll-export, it will report warning, so, just disable it
#endif

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