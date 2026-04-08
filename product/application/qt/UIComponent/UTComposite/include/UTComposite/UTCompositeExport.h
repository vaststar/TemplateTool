#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UTCOMPOSITE_LIB)
        #define UTCOMPOSITE_EXPORT
    #elif defined(UTCOMPOSITE_DLL)
        #define UTCOMPOSITE_EXPORT __declspec(dllexport)
    #else
        #define UTCOMPOSITE_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UTCOMPOSITE_DLL)
        #define UTCOMPOSITE_EXPORT __attribute__((visibility("default")))
    #else
        #define UTCOMPOSITE_EXPORT
    #endif
#endif
