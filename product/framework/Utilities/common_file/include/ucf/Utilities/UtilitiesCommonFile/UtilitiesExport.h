#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(Utilities_LIB)
        #define Utilities_EXPORT
    #elif defined(Utilities_DLL)
        #define Utilities_EXPORT __declspec(dllexport)
    #else
        #define Utilities_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(Utilities_DLL)
        #define Utilities_EXPORT __attribute__((visibility("default")))
    #else
        #define Utilities_EXPORT
    #endif
#endif
