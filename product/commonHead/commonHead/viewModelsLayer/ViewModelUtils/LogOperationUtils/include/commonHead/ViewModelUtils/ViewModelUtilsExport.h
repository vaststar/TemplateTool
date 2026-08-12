#pragma once

#if defined(VIEW_MODEL_UTILS_STATIC) && defined(VIEW_MODEL_UTILS_SHARED)
#  error "VIEW_MODEL_UTILS_STATIC and VIEW_MODEL_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(VIEW_MODEL_UTILS_STATIC)
#    define VIEW_MODEL_UTILS_API
#  elif defined(VIEW_MODEL_UTILS_SHARED)
#    define VIEW_MODEL_UTILS_API __declspec(dllexport)
#  else
#    define VIEW_MODEL_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(VIEW_MODEL_UTILS_STATIC)
#    define VIEW_MODEL_UTILS_API
#  else
#    define VIEW_MODEL_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
