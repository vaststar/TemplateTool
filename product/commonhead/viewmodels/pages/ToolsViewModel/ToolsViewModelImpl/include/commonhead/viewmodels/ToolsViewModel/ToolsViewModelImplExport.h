#pragma once

#if defined(TOOLS_VIEW_MODEL_IMPL_STATIC) && defined(TOOLS_VIEW_MODEL_IMPL_SHARED)
#  error "TOOLS_VIEW_MODEL_IMPL_STATIC and TOOLS_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(TOOLS_VIEW_MODEL_IMPL_STATIC)
#    define TOOLS_VIEW_MODEL_IMPL_API
#  elif defined(TOOLS_VIEW_MODEL_IMPL_SHARED)
#    define TOOLS_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define TOOLS_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(TOOLS_VIEW_MODEL_IMPL_STATIC)
#    define TOOLS_VIEW_MODEL_IMPL_API
#  else
#    define TOOLS_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
