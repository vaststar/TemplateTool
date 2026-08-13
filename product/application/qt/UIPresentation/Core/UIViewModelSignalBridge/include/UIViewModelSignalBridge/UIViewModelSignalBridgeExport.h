#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIViewModelSignalBridge_LIB)
        #define UIViewModelSignalBridge_EXPORT
    #elif defined(UIViewModelSignalBridge_DLL)
        #define UIViewModelSignalBridge_EXPORT __declspec(dllexport)
    #else
        #define UIViewModelSignalBridge_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIViewModelSignalBridge_DLL)
        #define UIViewModelSignalBridge_EXPORT __attribute__((visibility("default")))
    #else
        #define UIViewModelSignalBridge_EXPORT
    #endif
#endif
