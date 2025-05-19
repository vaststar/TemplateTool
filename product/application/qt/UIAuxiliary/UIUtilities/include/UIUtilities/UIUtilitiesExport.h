#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIUtilities_DLL)
        #define UIUtilities_EXPORT __declspec(dllexport)
    #else
        #define UIUtilities_EXPORT __declspec(dllimport)
    #endif
#else
    #define UIUtilities_EXPORT 
#endif
