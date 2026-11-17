#pragma once

#if defined(TRAY_VIEW_MODEL_IMPL_STATIC) && defined(TRAY_VIEW_MODEL_IMPL_SHARED)
#  error "TRAY_VIEW_MODEL_IMPL_STATIC and TRAY_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(TRAY_VIEW_MODEL_IMPL_STATIC)
#    define TRAY_VIEW_MODEL_IMPL_API
#  elif defined(TRAY_VIEW_MODEL_IMPL_SHARED)
#    define TRAY_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define TRAY_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(TRAY_VIEW_MODEL_IMPL_STATIC)
#    define TRAY_VIEW_MODEL_IMPL_API
#  else
#    define TRAY_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
