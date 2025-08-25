#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIResourceStringLoader_LIB)
        #define UIResourceStringLoader_EXPORT
    #elif defined(UIResourceStringLoader_DLL)
        #define UIResourceStringLoader_EXPORT __declspec(dllexport)
    #else
        #define UIResourceStringLoader_EXPORT __declspec(dllimport)
    #endif
#else
    #define UIResourceStringLoader_EXPORT 
#endif
