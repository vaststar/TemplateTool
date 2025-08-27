#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(NETWORKTYPE_LIB)
        #define NETWORKTYPE_EXPORT
    #elif defined(NETWORKTYPE_DLL)
        #define NETWORKTYPE_EXPORT __declspec(dllexport)
    #else
        #define NETWORKTYPE_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(NETWORKTYPE_DLL)
        #define NETWORKTYPE_EXPORT __attribute__((visibility("default")))
    #else
        #define NETWORKTYPE_EXPORT
    #endif
#endif
