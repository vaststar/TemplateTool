#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIDataStruct_LIB)
        #define UIDataStruct_EXPORT
    #elif defined(UIDataStruct_DLL)
        #define UIDataStruct_EXPORT __declspec(dllexport)
    #else
        #define UIDataStruct_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIDataStruct_DLL)
        #define UIDataStruct_EXPORT __attribute__((visibility("default")))
    #else
        #define UIDataStruct_EXPORT
    #endif
#endif
