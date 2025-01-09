#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIDataStruct_DLL)
        #define UIDataStruct_EXPORT __declspec(dllexport)
    #else
        #define UIDataStruct_EXPORT __declspec(dllimport)
    #endif
#else
    #define UIDataStruct_EXPORT 
#endif
