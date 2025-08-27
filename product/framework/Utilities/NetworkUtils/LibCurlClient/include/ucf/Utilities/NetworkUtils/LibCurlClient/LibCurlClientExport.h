#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #if defined(LIBCURLCLIENT_LIB)
        #define LIBCURLCLIENT_EXPORT
    #elif defined(LIBCURLCLIENT_DLL)
        #define LIBCURLCLIENT_EXPORT __declspec(dllexport)
    #else
        #define LIBCURLCLIENT_EXPORT __declspec(dllimport)
    #endif
#else
    #if defined(LIBCURLCLIENT_DLL)
        #define LIBCURLCLIENT_EXPORT __attribute__((visibility("default")))
    #else
        #define LIBCURLCLIENT_EXPORT
    #endif
#endif
