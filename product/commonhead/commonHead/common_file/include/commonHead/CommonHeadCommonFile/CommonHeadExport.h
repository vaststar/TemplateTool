#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(COMMONHEAD_DLL)
        #define COMMONHEAD_EXPORT __declspec(dllexport)
    #else
        #define COMMONHEAD_EXPORT __declspec(dllimport)
    #endif
#else
    #define COMMONHEAD_EXPORT 
#endif
