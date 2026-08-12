#pragma once

#if defined(SCREENSHOT_VIEW_MODEL_IMPL_STATIC) && defined(SCREENSHOT_VIEW_MODEL_IMPL_SHARED)
#  error "SCREENSHOT_VIEW_MODEL_IMPL_STATIC and SCREENSHOT_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(SCREENSHOT_VIEW_MODEL_IMPL_STATIC)
#    define SCREENSHOT_VIEW_MODEL_IMPL_API
#  elif defined(SCREENSHOT_VIEW_MODEL_IMPL_SHARED)
#    define SCREENSHOT_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define SCREENSHOT_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(SCREENSHOT_VIEW_MODEL_IMPL_STATIC)
#    define SCREENSHOT_VIEW_MODEL_IMPL_API
#  else
#    define SCREENSHOT_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
