#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(Utilities_DLL)
        #define Utilities_EXPORT __declspec(dllexport)
    #else
        #define Utilities_EXPORT __declspec(dllimport)
    #endif
#else
    #define Utilities_EXPORT 
#endif
