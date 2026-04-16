#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(Agents_LIB)
        #define Agents_EXPORT
    #elif defined(Agents_DLL)
        #define Agents_EXPORT __declspec(dllexport)
    #else
        #define Agents_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(Agents_DLL)
        #define Agents_EXPORT __attribute__((visibility("default")))
    #else
        #define Agents_EXPORT
    #endif
#endif
