#pragma once

#if defined(CLIENT_INFO_VIEW_MODEL_IMPL_STATIC) && defined(CLIENT_INFO_VIEW_MODEL_IMPL_SHARED)
#  error "CLIENT_INFO_VIEW_MODEL_IMPL_STATIC and CLIENT_INFO_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(CLIENT_INFO_VIEW_MODEL_IMPL_STATIC)
#    define CLIENT_INFO_VIEW_MODEL_IMPL_API
#  elif defined(CLIENT_INFO_VIEW_MODEL_IMPL_SHARED)
#    define CLIENT_INFO_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define CLIENT_INFO_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(CLIENT_INFO_VIEW_MODEL_IMPL_STATIC)
#    define CLIENT_INFO_VIEW_MODEL_IMPL_API
#  else
#    define CLIENT_INFO_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
