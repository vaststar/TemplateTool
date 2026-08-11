#pragma once

#if defined(SETTINGS_VIEW_MODEL_IMPL_STATIC) && defined(SETTINGS_VIEW_MODEL_IMPL_SHARED)
#  error "SETTINGS_VIEW_MODEL_IMPL_STATIC and SETTINGS_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(SETTINGS_VIEW_MODEL_IMPL_STATIC)
#    define SETTINGS_VIEW_MODEL_IMPL_API
#  elif defined(SETTINGS_VIEW_MODEL_IMPL_SHARED)
#    define SETTINGS_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define SETTINGS_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(SETTINGS_VIEW_MODEL_IMPL_STATIC)
#    define SETTINGS_VIEW_MODEL_IMPL_API
#  else
#    define SETTINGS_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
