#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIResourceLoader_LIB)
        #define UIResourceLoader_EXPORT
    #elif defined(UIResourceLoader_DLL)
        #define UIResourceLoader_EXPORT __declspec(dllexport)
    #else
        #define UIResourceLoader_EXPORT __declspec(dllimport)
    #endif
#else
    #define UIResourceLoader_EXPORT 
#endif
