#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(APPRUNNER_DLL)
        #define APPRUNNER_EXPORT __declspec(dllexport)
    #else
        #define APPRUNNER_EXPORT __declspec(dllimport)
    #endif
#else
    #define APPRUNNER_EXPORT 
#endif
