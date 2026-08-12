#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UISingleInstance_LIB)
        #define UISingleInstance_EXPORT
    #elif defined(UISingleInstance_DLL)
        #define UISingleInstance_EXPORT __declspec(dllexport)
    #else
        #define UISingleInstance_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UISingleInstance_DLL)
        #define UISingleInstance_EXPORT __attribute__((visibility("default")))
    #else
        #define UISingleInstance_EXPORT
    #endif
#endif
