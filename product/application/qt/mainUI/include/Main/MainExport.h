#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(Main_DLL)
        #define Main_EXPORT __declspec(dllexport)
    #else
        #define Main_EXPORT __declspec(dllimport)
    #endif
#else
    #define Main_EXPORT 
#endif
