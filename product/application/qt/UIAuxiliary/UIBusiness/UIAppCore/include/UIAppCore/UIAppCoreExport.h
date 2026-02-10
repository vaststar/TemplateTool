#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIAppCore_LIB)
        #define UIAppCore_EXPORT
    #elif defined(UIAppCore_DLL)
        #define UIAppCore_EXPORT __declspec(dllexport)
    #else
        #define UIAppCore_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIAppCore_DLL)
        #define UIAppCore_EXPORT __attribute__((visibility("default")))
    #else
        #define UIAppCore_EXPORT
    #endif
#endif
