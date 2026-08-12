#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIViewCore_LIB)
        #define UIViewCore_EXPORT
    #elif defined(UIViewCore_DLL)
        #define UIViewCore_EXPORT __declspec(dllexport)
    #else
        #define UIViewCore_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIViewCore_DLL)
        #define UIViewCore_EXPORT __attribute__((visibility("default")))
    #else
        #define UIViewCore_EXPORT
    #endif
#endif
