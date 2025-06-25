#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(AppUI_LIB)
        #define AppUI_EXPORT
    #elif defined(AppUI_DLL)
        #define AppUI_EXPORT __declspec(dllexport)
    #else
        #define AppUI_EXPORT __declspec(dllimport)
    #endif
#else
    #define AppUI_EXPORT 
#endif
