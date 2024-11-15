#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UICore_DLL)
        #define UICore_EXPORT __declspec(dllexport)
    #else
        #define UICore_EXPORT __declspec(dllimport)
    #endif
#else
    #define UICore_EXPORT 
#endif
