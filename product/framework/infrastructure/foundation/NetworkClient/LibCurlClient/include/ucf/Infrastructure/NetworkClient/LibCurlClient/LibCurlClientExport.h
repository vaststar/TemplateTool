#pragma once

// Per-module export macro for LibCurlClient.
//   LIB_CURL_CLIENT_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   LIB_CURL_CLIENT_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(LIB_CURL_CLIENT_STATIC) && defined(LIB_CURL_CLIENT_SHARED)
#  error "LIB_CURL_CLIENT_STATIC and LIB_CURL_CLIENT_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(LIB_CURL_CLIENT_STATIC)
#    define LIB_CURL_CLIENT_API
#  elif defined(LIB_CURL_CLIENT_SHARED)
#    define LIB_CURL_CLIENT_API __declspec(dllexport)
#  else
#    define LIB_CURL_CLIENT_API __declspec(dllimport)
#  endif
#else
#  if defined(LIB_CURL_CLIENT_STATIC)
#    define LIB_CURL_CLIENT_API
#  else
#    define LIB_CURL_CLIENT_API __attribute__((visibility("default")))
#  endif
#endif
