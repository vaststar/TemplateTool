#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIComponentBase_LIB)
        #define UIComponentBase_EXPORT
    #elif defined(UIComponentBase_DLL)
        #define UIComponentBase_EXPORT __declspec(dllexport)
    #else
        #define UIComponentBase_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIComponentBase_DLL)
        #define UIComponentBase_EXPORT __attribute__((visibility("default")))
    #else
        #define UIComponentBase_EXPORT
    #endif
#endif
