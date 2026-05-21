#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(Infrastructure_LIB)
        #define Infrastructure_EXPORT
    #elif defined(Infrastructure_DLL)
        #define Infrastructure_EXPORT __declspec(dllexport)
    #else
        #define Infrastructure_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(Infrastructure_DLL)
        #define Infrastructure_EXPORT __attribute__((visibility("default")))
    #else
        #define Infrastructure_EXPORT
    #endif
#endif
