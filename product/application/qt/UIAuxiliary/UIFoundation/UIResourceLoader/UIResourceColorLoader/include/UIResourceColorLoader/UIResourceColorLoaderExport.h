#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIResourceColorLoader_LIB)
        #define UIResourceColorLoader_EXPORT
    #elif defined(UIResourceColorLoader_DLL)
        #define UIResourceColorLoader_EXPORT __declspec(dllexport)
    #else
        #define UIResourceColorLoader_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIResourceColorLoader_DLL)
        #define UIResourceColorLoader_EXPORT __attribute__((visibility("default")))
    #else
        #define UIResourceColorLoader_EXPORT
    #endif
#endif
