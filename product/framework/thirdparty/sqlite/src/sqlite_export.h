#ifndef ThreadPoolExport_h__
#define ThreadPoolExport_h__

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4251)//for template class member with dll-export, it will report warning, so, just disable it
    #if defined(SQLITE_BUILD_LIB) // for static library
        #define SQLITE_EXPORT
    #elif defined(SQLITE_BUILD_DLL) // for dynamic library
        #define SQLITE_EXPORT __declspec(dllexport)
    #else
        #define SQLITE_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(SQLITE_BUILD_DLL)
        #define SQLITE_EXPORT __attribute__((visibility("default")))
    #else
        #define SQLITE_EXPORT
    #endif
#endif

#endif

#define SQLITE_API SQLITE_EXPORT