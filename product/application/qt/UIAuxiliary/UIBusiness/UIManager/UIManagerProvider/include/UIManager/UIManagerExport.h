#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIManager_LIB)
        #define UIManager_EXPORT
    #elif defined(UIManager_DLL)
        #define UIManager_EXPORT __declspec(dllexport)
    #else
        #define UIManager_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIManager_DLL)
        #define UIManager_EXPORT __attribute__((visibility("default")))
    #else
        #define UIManager_EXPORT
    #endif
#endif
