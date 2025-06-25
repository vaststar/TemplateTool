#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(AppContext_LIB)
        #define AppContext_EXPORT
    #elif defined(AppContext_DLL)
        #define AppContext_EXPORT __declspec(dllexport)
    #else
        #define AppContext_EXPORT __declspec(dllimport)
    #endif
#else
    #define AppContext_EXPORT 
#endif
