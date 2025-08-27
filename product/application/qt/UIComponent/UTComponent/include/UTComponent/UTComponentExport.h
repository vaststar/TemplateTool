#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UTCOMPONENT_LIB)
        #define UTCOMPONENT_EXPORT
    #elif defined(UTCOMPONENT_DLL)
        #define UTCOMPONENT_EXPORT __declspec(dllexport)
    #else
        #define UTCOMPONENT_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UTCOMPONENT_DLL)
        #define UTCOMPONENT_EXPORT __attribute__((visibility("default")))
    #else
        #define UTCOMPONENT_EXPORT
    #endif
#endif
