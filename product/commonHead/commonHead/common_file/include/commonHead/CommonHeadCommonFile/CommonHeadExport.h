#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(COMMONHEAD_LIB)
        #define COMMONHEAD_EXPORT
    #elif defined(COMMONHEAD_DLL)
        #define COMMONHEAD_EXPORT __declspec(dllexport)
    #else
        #define COMMONHEAD_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(COMMONHEAD_DLL)
        #define COMMONHEAD_EXPORT __attribute__((visibility("default")))
    #else
        #define COMMONHEAD_EXPORT
    #endif
#endif
