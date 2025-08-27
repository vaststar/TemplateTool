#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UICore_LIB)
        #define UICore_EXPORT
    #elif defined(UICore_DLL)
        #define UICore_EXPORT __declspec(dllexport)
    #else
        #define UICore_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UICore_DLL)
        #define UICore_EXPORT __attribute__((visibility("default")))
    #else
        #define UICore_EXPORT
    #endif
#endif
