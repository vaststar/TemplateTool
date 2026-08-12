#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIIPCChannel_LIB)
        #define UIIPCChannel_EXPORT
    #elif defined(UIIPCChannel_DLL)
        #define UIIPCChannel_EXPORT __declspec(dllexport)
    #else
        #define UIIPCChannel_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIIPCChannel_DLL)
        #define UIIPCChannel_EXPORT __attribute__((visibility("default")))
    #else
        #define UIIPCChannel_EXPORT
    #endif
#endif
