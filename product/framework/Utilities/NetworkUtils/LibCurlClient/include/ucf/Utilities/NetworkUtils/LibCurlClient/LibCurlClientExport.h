#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(LIBCURLCLIENT_DLL)
        #define LIBCURLCLIENT_EXPORT __declspec(dllexport)
    #else
        #define LIBCURLCLIENT_EXPORT __declspec(dllimport)
    #endif
#else
    #define LIBCURLCLIENT_EXPORT 
#endif
