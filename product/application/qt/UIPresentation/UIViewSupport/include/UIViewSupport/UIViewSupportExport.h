#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIViewSupport_LIB)
        #define UIViewSupport_EXPORT
    #elif defined(UIViewSupport_DLL)
        #define UIViewSupport_EXPORT __declspec(dllexport)
    #else
        #define UIViewSupport_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIViewSupport_DLL)
        #define UIViewSupport_EXPORT __attribute__((visibility("default")))
    #else
        #define UIViewSupport_EXPORT
    #endif
#endif
