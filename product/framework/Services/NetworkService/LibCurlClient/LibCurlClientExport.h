#ifndef LibCurlClientExport_h__
#define LibCurlClientExport_h__

#ifdef LIBCURLCLIENT_DLL
#ifdef LIBCURLCLIENT_LIB
#define LIBCURLCLIENT_EXPORT _declspec(dllexport)
#else
#define LIBCURLCLIENT_EXPORT _declspec(dllimport)
#endif
#else
#define LIBCURLCLIENT_EXPORT 
#endif

#endif //LibCurlClientExport_h__