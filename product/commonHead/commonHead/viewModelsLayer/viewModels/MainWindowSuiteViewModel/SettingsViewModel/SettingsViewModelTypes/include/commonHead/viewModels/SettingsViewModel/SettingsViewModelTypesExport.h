#pragma once

#if defined(SETTINGS_VIEW_MODEL_TYPES_STATIC) && defined(SETTINGS_VIEW_MODEL_TYPES_SHARED)
#  error "SETTINGS_VIEW_MODEL_TYPES_STATIC and SETTINGS_VIEW_MODEL_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(SETTINGS_VIEW_MODEL_TYPES_STATIC)
#    define SETTINGS_VIEW_MODEL_TYPES_API
#  elif defined(SETTINGS_VIEW_MODEL_TYPES_SHARED)
#    define SETTINGS_VIEW_MODEL_TYPES_API __declspec(dllexport)
#  else
#    define SETTINGS_VIEW_MODEL_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(SETTINGS_VIEW_MODEL_TYPES_STATIC)
#    define SETTINGS_VIEW_MODEL_TYPES_API
#  else
#    define SETTINGS_VIEW_MODEL_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
