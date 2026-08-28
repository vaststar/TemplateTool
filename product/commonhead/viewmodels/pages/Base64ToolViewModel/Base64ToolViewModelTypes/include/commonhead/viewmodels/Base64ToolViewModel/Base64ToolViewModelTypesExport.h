#pragma once

#if defined(BASE64_TOOL_VIEW_MODEL_TYPES_STATIC) && defined(BASE64_TOOL_VIEW_MODEL_TYPES_SHARED)
#  error "BASE64_TOOL_VIEW_MODEL_TYPES_STATIC and BASE64_TOOL_VIEW_MODEL_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(BASE64_TOOL_VIEW_MODEL_TYPES_STATIC)
#    define BASE64_TOOL_VIEW_MODEL_TYPES_API
#  elif defined(BASE64_TOOL_VIEW_MODEL_TYPES_SHARED)
#    define BASE64_TOOL_VIEW_MODEL_TYPES_API __declspec(dllexport)
#  else
#    define BASE64_TOOL_VIEW_MODEL_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(BASE64_TOOL_VIEW_MODEL_TYPES_STATIC)
#    define BASE64_TOOL_VIEW_MODEL_TYPES_API
#  else
#    define BASE64_TOOL_VIEW_MODEL_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
