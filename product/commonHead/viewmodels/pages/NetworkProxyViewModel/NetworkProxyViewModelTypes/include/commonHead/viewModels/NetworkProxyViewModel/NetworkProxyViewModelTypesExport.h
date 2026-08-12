#pragma once

#if defined(NETWORK_PROXY_VIEW_MODEL_TYPES_STATIC) && defined(NETWORK_PROXY_VIEW_MODEL_TYPES_SHARED)
#  error "NETWORK_PROXY_VIEW_MODEL_TYPES_STATIC and NETWORK_PROXY_VIEW_MODEL_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(NETWORK_PROXY_VIEW_MODEL_TYPES_STATIC)
#    define NETWORK_PROXY_VIEW_MODEL_TYPES_API
#  elif defined(NETWORK_PROXY_VIEW_MODEL_TYPES_SHARED)
#    define NETWORK_PROXY_VIEW_MODEL_TYPES_API __declspec(dllexport)
#  else
#    define NETWORK_PROXY_VIEW_MODEL_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(NETWORK_PROXY_VIEW_MODEL_TYPES_STATIC)
#    define NETWORK_PROXY_VIEW_MODEL_TYPES_API
#  else
#    define NETWORK_PROXY_VIEW_MODEL_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
