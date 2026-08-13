#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIView_LIB)
        #define UIView_EXPORT
    #elif defined(UIView_DLL)
        #define UIView_EXPORT __declspec(dllexport)
    #else
        #define UIView_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIView_DLL)
        #define UIView_EXPORT __attribute__((visibility("default")))
    #else
        #define UIView_EXPORT
    #endif
#endif
