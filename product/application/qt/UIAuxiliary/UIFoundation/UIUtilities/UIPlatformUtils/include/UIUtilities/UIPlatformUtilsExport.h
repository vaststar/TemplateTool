#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIPlatformUtils_LIB)
        #define UIPlatformUtils_EXPORT
    #elif defined(UIPlatformUtils_DLL)
        #define UIPlatformUtils_EXPORT __declspec(dllexport)
    #else
        #define UIPlatformUtils_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIPlatformUtils_DLL)
        #define UIPlatformUtils_EXPORT __attribute__((visibility("default")))
    #else
        #define UIPlatformUtils_EXPORT
    #endif
#endif
