#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIResourceLoaderManager_LIB)
        #define UIResourceLoaderManager_EXPORT
    #elif defined(UIResourceLoaderManager_DLL)
        #define UIResourceLoaderManager_EXPORT __declspec(dllexport)
    #else
        #define UIResourceLoaderManager_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIResourceLoaderManager_DLL)
        #define UIResourceLoaderManager_EXPORT __attribute__((visibility("default")))
    #else
        #define UIResourceLoaderManager_EXPORT
    #endif
#endif
