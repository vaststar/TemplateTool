#pragma once

// Per-module export macro for FeatureSettingsServiceTypes.
//   FEATURE_SETTINGS_SERVICE_TYPES_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   FEATURE_SETTINGS_SERVICE_TYPES_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(FEATURE_SETTINGS_SERVICE_TYPES_STATIC) && defined(FEATURE_SETTINGS_SERVICE_TYPES_SHARED)
#  error "FEATURE_SETTINGS_SERVICE_TYPES_STATIC and FEATURE_SETTINGS_SERVICE_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(FEATURE_SETTINGS_SERVICE_TYPES_STATIC)
#    define FEATURE_SETTINGS_SERVICE_TYPES_API
#  elif defined(FEATURE_SETTINGS_SERVICE_TYPES_SHARED)
#    define FEATURE_SETTINGS_SERVICE_TYPES_API __declspec(dllexport)
#  else
#    define FEATURE_SETTINGS_SERVICE_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(FEATURE_SETTINGS_SERVICE_TYPES_STATIC)
#    define FEATURE_SETTINGS_SERVICE_TYPES_API
#  else
#    define FEATURE_SETTINGS_SERVICE_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
