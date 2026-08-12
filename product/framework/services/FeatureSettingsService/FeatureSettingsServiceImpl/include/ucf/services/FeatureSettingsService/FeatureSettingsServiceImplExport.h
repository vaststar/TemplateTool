#pragma once

#if defined(FEATURE_SETTINGS_SERVICE_IMPL_STATIC) && defined(FEATURE_SETTINGS_SERVICE_IMPL_SHARED)
#  error "FEATURE_SETTINGS_SERVICE_IMPL_STATIC and FEATURE_SETTINGS_SERVICE_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(FEATURE_SETTINGS_SERVICE_IMPL_STATIC)
#    define FEATURE_SETTINGS_SERVICE_IMPL_API
#  elif defined(FEATURE_SETTINGS_SERVICE_IMPL_SHARED)
#    define FEATURE_SETTINGS_SERVICE_IMPL_API __declspec(dllexport)
#  else
#    define FEATURE_SETTINGS_SERVICE_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(FEATURE_SETTINGS_SERVICE_IMPL_STATIC)
#    define FEATURE_SETTINGS_SERVICE_IMPL_API
#  else
#    define FEATURE_SETTINGS_SERVICE_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
