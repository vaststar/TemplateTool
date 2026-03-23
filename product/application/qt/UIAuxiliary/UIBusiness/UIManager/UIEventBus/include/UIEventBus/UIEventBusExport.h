#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIEventBus_LIB)
        #define UIEventBus_EXPORT
    #elif defined(UIEventBus_DLL)
        #define UIEventBus_EXPORT __declspec(dllexport)
    #else
        #define UIEventBus_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIEventBus_DLL)
        #define UIEventBus_EXPORT __attribute__((visibility("default")))
    #else
        #define UIEventBus_EXPORT
    #endif
#endif
