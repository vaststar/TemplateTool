#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(DATABASEWRAPPER_DLL)
        #define DATABASEWRAPPER_EXPORT __declspec(dllexport)
    #else
        #define DATABASEWRAPPER_EXPORT __declspec(dllimport)
    #endif
#else
    #define DATABASEWRAPPER_EXPORT 
#endif
