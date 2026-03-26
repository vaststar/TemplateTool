#pragma once

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4251)//for template class member with dll-export, it will report warning, so, just disable it
    #if defined(SERVICE_LIB) // for static library
        #define SERVICE_EXPORT
    #elif defined(SERVICE_DLL) // for dynamic library
        #define SERVICE_EXPORT __declspec(dllexport)
    #else
        #define SERVICE_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(SERVICE_DLL)
        #define SERVICE_EXPORT __attribute__((visibility("default")))
    #else
        #define SERVICE_EXPORT
    #endif
#endif
