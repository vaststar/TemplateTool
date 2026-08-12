#pragma once

#if defined(INVOCATION_VIEW_MODEL_IMPL_STATIC) && defined(INVOCATION_VIEW_MODEL_IMPL_SHARED)
#  error "INVOCATION_VIEW_MODEL_IMPL_STATIC and INVOCATION_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(INVOCATION_VIEW_MODEL_IMPL_STATIC)
#    define INVOCATION_VIEW_MODEL_IMPL_API
#  elif defined(INVOCATION_VIEW_MODEL_IMPL_SHARED)
#    define INVOCATION_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define INVOCATION_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(INVOCATION_VIEW_MODEL_IMPL_STATIC)
#    define INVOCATION_VIEW_MODEL_IMPL_API
#  else
#    define INVOCATION_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
