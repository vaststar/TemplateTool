#ifndef ThreadPoolExport_h__
#define ThreadPoolExport_h__

#ifdef Utilities_DLL
#ifdef Utilities_LIB
#define THREADPOOL_EXPORT _declspec(dllexport)
#else
#define THREADPOOL_EXPORT _declspec(dllimport)
#endif
#else
#define THREADPOOL_EXPORT 
#endif

#endif //ThreadPoolExport_h__