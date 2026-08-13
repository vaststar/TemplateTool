#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIQmlUtilities_LIB)
        #define UIQmlUtilities_EXPORT
    #elif defined(UIQmlUtilities_DLL)
        #define UIQmlUtilities_EXPORT __declspec(dllexport)
    #else
        #define UIQmlUtilities_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIQmlUtilities_DLL)
        #define UIQmlUtilities_EXPORT __attribute__((visibility("default")))
    #else
        #define UIQmlUtilities_EXPORT
    #endif
#endif
