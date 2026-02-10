#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIUtilities_LIB)
        #define UIUtilities_EXPORT
    #elif defined(UIUtilities_DLL)
        #define UIUtilities_EXPORT __declspec(dllexport)
    #else
        #define UIUtilities_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIUtilities_DLL)
        #define UIUtilities_EXPORT __attribute__((visibility("default")))
    #else
        #define UIUtilities_EXPORT
    #endif
#endif
