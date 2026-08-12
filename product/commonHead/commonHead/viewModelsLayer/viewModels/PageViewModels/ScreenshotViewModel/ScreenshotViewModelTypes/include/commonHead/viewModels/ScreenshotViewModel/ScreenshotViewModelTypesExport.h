#pragma once

#if defined(SCREENSHOT_VIEW_MODEL_TYPES_STATIC) && defined(SCREENSHOT_VIEW_MODEL_TYPES_SHARED)
#  error "SCREENSHOT_VIEW_MODEL_TYPES_STATIC and SCREENSHOT_VIEW_MODEL_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(SCREENSHOT_VIEW_MODEL_TYPES_STATIC)
#    define SCREENSHOT_VIEW_MODEL_TYPES_API
#  elif defined(SCREENSHOT_VIEW_MODEL_TYPES_SHARED)
#    define SCREENSHOT_VIEW_MODEL_TYPES_API __declspec(dllexport)
#  else
#    define SCREENSHOT_VIEW_MODEL_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(SCREENSHOT_VIEW_MODEL_TYPES_STATIC)
#    define SCREENSHOT_VIEW_MODEL_TYPES_API
#  else
#    define SCREENSHOT_VIEW_MODEL_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
