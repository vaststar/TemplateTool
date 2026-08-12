#pragma once

#if defined(TOOLS_VIEW_MODEL_TYPES_STATIC) && defined(TOOLS_VIEW_MODEL_TYPES_SHARED)
#  error "TOOLS_VIEW_MODEL_TYPES_STATIC and TOOLS_VIEW_MODEL_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(TOOLS_VIEW_MODEL_TYPES_STATIC)
#    define TOOLS_VIEW_MODEL_TYPES_API
#  elif defined(TOOLS_VIEW_MODEL_TYPES_SHARED)
#    define TOOLS_VIEW_MODEL_TYPES_API __declspec(dllexport)
#  else
#    define TOOLS_VIEW_MODEL_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(TOOLS_VIEW_MODEL_TYPES_STATIC)
#    define TOOLS_VIEW_MODEL_TYPES_API
#  else
#    define TOOLS_VIEW_MODEL_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
