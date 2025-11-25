#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIResourceAssetLoader_LIB)
        #define UIResourceAssetLoader_EXPORT
    #elif defined(UIResourceAssetLoader_DLL)
        #define UIResourceAssetLoader_EXPORT __declspec(dllexport)
    #else
        #define UIResourceAssetLoader_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIResourceAssetLoader_DLL)
        #define UIResourceAssetLoader_EXPORT __attribute__((visibility("default")))
    #else
        #define UIResourceAssetLoader_EXPORT
    #endif
#endif
