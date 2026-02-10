#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(UIResourceFontLoader_LIB)
        #define UIResourceFontLoader_EXPORT
    #elif defined(UIResourceFontLoader_DLL)
        #define UIResourceFontLoader_EXPORT __declspec(dllexport)
    #else
        #define UIResourceFontLoader_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(UIResourceFontLoader_DLL)
        #define UIResourceFontLoader_EXPORT __attribute__((visibility("default")))
    #else
        #define UIResourceFontLoader_EXPORT
    #endif
#endif
