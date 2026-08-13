#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIEvents_LIB)
        #define UIEvents_EXPORT
    #elif defined(UIEvents_DLL)
        #define UIEvents_EXPORT __declspec(dllexport)
    #else
        #define UIEvents_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIEvents_DLL)
        #define UIEvents_EXPORT __attribute__((visibility("default")))
    #else
        #define UIEvents_EXPORT
    #endif
#endif
