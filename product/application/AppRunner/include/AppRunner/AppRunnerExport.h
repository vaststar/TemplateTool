#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(APPRUNNER_LIB)
        #define APPRUNNER_EXPORT
    #elif defined(APPRUNNER_DLL)
        #define APPRUNNER_EXPORT __declspec(dllexport)
    #else
        #define APPRUNNER_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(APPRUNNER_DLL)
        #define APPRUNNER_EXPORT __attribute__((visibility("default")))
    #else
        #define APPRUNNER_EXPORT
    #endif
#endif
