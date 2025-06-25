#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIFabrication_LIB)
        #define UIFabrication_EXPORT
    #elif defined(UIFabrication_DLL)
        #define UIFabrication_EXPORT __declspec(dllexport)
    #else
        #define UIFabrication_EXPORT __declspec(dllimport)
    #endif
#else
    #define UIFabrication_EXPORT 
#endif
