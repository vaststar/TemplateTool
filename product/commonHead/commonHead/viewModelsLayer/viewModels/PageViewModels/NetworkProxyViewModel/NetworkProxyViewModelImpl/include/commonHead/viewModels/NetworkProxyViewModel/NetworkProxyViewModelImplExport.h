#pragma once

#if defined(NETWORK_PROXY_VIEW_MODEL_IMPL_STATIC) && defined(NETWORK_PROXY_VIEW_MODEL_IMPL_SHARED)
#  error "NETWORK_PROXY_VIEW_MODEL_IMPL_STATIC and NETWORK_PROXY_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(NETWORK_PROXY_VIEW_MODEL_IMPL_STATIC)
#    define NETWORK_PROXY_VIEW_MODEL_IMPL_API
#  elif defined(NETWORK_PROXY_VIEW_MODEL_IMPL_SHARED)
#    define NETWORK_PROXY_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define NETWORK_PROXY_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(NETWORK_PROXY_VIEW_MODEL_IMPL_STATIC)
#    define NETWORK_PROXY_VIEW_MODEL_IMPL_API
#  else
#    define NETWORK_PROXY_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
