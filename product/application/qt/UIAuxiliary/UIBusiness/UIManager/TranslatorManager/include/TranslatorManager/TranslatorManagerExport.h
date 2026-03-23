#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(TranslatorManager_LIB)
        #define TranslatorManager_EXPORT
    #elif defined(TranslatorManager_DLL)
        #define TranslatorManager_EXPORT __declspec(dllexport)
    #else
        #define TranslatorManager_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(TranslatorManager_DLL)
        #define TranslatorManager_EXPORT __attribute__((visibility("default")))
    #else
        #define TranslatorManager_EXPORT
    #endif
#endif
