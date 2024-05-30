#ifndef CommonHeadExport_h__
#define CommonHeadExport_h__

#ifdef COMMONHEAD_DLL
#ifdef COMMONHEAD_LIB
#define COMMONHEAD_EXPORT _declspec(dllexport)
#else
#define COMMONHEAD_EXPORT _declspec(dllimport)
#endif
#else
#define COMMONHEAD_EXPORT 
#endif

#endif //CommonHeadExport_h__