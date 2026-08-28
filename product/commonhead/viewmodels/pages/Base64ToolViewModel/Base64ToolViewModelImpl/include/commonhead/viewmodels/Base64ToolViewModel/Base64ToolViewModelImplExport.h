#pragma once

#if defined(BASE64_TOOL_VIEW_MODEL_IMPL_STATIC) && defined(BASE64_TOOL_VIEW_MODEL_IMPL_SHARED)
#  error "BASE64_TOOL_VIEW_MODEL_IMPL_STATIC and BASE64_TOOL_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(BASE64_TOOL_VIEW_MODEL_IMPL_STATIC)
#    define BASE64_TOOL_VIEW_MODEL_IMPL_API
#  elif defined(BASE64_TOOL_VIEW_MODEL_IMPL_SHARED)
#    define BASE64_TOOL_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define BASE64_TOOL_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(BASE64_TOOL_VIEW_MODEL_IMPL_STATIC)
#    define BASE64_TOOL_VIEW_MODEL_IMPL_API
#  else
#    define BASE64_TOOL_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
