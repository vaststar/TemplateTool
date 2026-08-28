#pragma once

#if defined(UUID_TOOL_VIEW_MODEL_IMPL_STATIC) && defined(UUID_TOOL_VIEW_MODEL_IMPL_SHARED)
#  error "UUID_TOOL_VIEW_MODEL_IMPL_STATIC and UUID_TOOL_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(UUID_TOOL_VIEW_MODEL_IMPL_STATIC)
#    define UUID_TOOL_VIEW_MODEL_IMPL_API
#  elif defined(UUID_TOOL_VIEW_MODEL_IMPL_SHARED)
#    define UUID_TOOL_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define UUID_TOOL_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(UUID_TOOL_VIEW_MODEL_IMPL_STATIC)
#    define UUID_TOOL_VIEW_MODEL_IMPL_API
#  else
#    define UUID_TOOL_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
