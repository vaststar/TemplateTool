#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIWindowUtilities_LIB)
        #define UIWindowUtilities_EXPORT
    #elif defined(UIWindowUtilities_DLL)
        #define UIWindowUtilities_EXPORT __declspec(dllexport)
    #else
        #define UIWindowUtilities_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIWindowUtilities_DLL)
        #define UIWindowUtilities_EXPORT __attribute__((visibility("default")))
    #else
        #define UIWindowUtilities_EXPORT
    #endif
#endif
