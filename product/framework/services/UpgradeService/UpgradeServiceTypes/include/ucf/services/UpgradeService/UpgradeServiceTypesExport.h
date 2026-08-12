#pragma once

// Per-module export macro for UpgradeServiceTypes.
//   UPGRADE_SERVICE_TYPES_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   UPGRADE_SERVICE_TYPES_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(UPGRADE_SERVICE_TYPES_STATIC) && defined(UPGRADE_SERVICE_TYPES_SHARED)
#  error "UPGRADE_SERVICE_TYPES_STATIC and UPGRADE_SERVICE_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(UPGRADE_SERVICE_TYPES_STATIC)
#    define UPGRADE_SERVICE_TYPES_API
#  elif defined(UPGRADE_SERVICE_TYPES_SHARED)
#    define UPGRADE_SERVICE_TYPES_API __declspec(dllexport)
#  else
#    define UPGRADE_SERVICE_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(UPGRADE_SERVICE_TYPES_STATIC)
#    define UPGRADE_SERVICE_TYPES_API
#  else
#    define UPGRADE_SERVICE_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
