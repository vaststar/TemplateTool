#pragma once

#if defined(JSON_TOOL_VIEW_MODEL_IMPL_STATIC) && defined(JSON_TOOL_VIEW_MODEL_IMPL_SHARED)
#  error "JSON_TOOL_VIEW_MODEL_IMPL_STATIC and JSON_TOOL_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(JSON_TOOL_VIEW_MODEL_IMPL_STATIC)
#    define JSON_TOOL_VIEW_MODEL_IMPL_API
#  elif defined(JSON_TOOL_VIEW_MODEL_IMPL_SHARED)
#    define JSON_TOOL_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define JSON_TOOL_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(JSON_TOOL_VIEW_MODEL_IMPL_STATIC)
#    define JSON_TOOL_VIEW_MODEL_IMPL_API
#  else
#    define JSON_TOOL_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
