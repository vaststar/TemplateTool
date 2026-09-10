#pragma once

#if defined(NETWORK_TRANSPORT_UTILS_STATIC) \
    && defined(NETWORK_TRANSPORT_UTILS_SHARED)
#  error "NETWORK_TRANSPORT_UTILS_STATIC and NETWORK_TRANSPORT_UTILS_SHARED cannot both be defined"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(NETWORK_TRANSPORT_UTILS_STATIC)
#    define NETWORK_TRANSPORT_UTILS_API
#  elif defined(NETWORK_TRANSPORT_UTILS_SHARED)
#    define NETWORK_TRANSPORT_UTILS_API __declspec(dllexport)
#  else
#    define NETWORK_TRANSPORT_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(NETWORK_TRANSPORT_UTILS_STATIC)
#    define NETWORK_TRANSPORT_UTILS_API
#  else
#    define NETWORK_TRANSPORT_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
