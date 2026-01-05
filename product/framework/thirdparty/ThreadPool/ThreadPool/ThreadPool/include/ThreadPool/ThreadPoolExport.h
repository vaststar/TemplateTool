#ifndef ThreadPoolExport_h__
#define ThreadPoolExport_h__

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4251)//for template class member with dll-export, it will report warning, so, just disable it
    #if defined(THREADPOOL_LIB) // for static library
        #define THREADPOOL_EXPORT
    #elif defined(THREADPOOL_DLL) // for dynamic library
        #define THREADPOOL_EXPORT __declspec(dllexport)
    #else
        #define THREADPOOL_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(THREADPOOL_DLL)
        #define THREADPOOL_EXPORT __attribute__((visibility("default")))
    #else
        #define THREADPOOL_EXPORT
    #endif
#endif

#endif
