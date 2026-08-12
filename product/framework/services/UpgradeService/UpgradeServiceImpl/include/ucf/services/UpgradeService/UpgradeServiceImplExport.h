#pragma once

// Per-module export macro for UpgradeServiceImpl.
//   UPGRADE_SERVICE_IMPL_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   UPGRADE_SERVICE_IMPL_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(UPGRADE_SERVICE_IMPL_STATIC) && defined(UPGRADE_SERVICE_IMPL_SHARED)
#  error "UPGRADE_SERVICE_IMPL_STATIC and UPGRADE_SERVICE_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(UPGRADE_SERVICE_IMPL_STATIC)
#    define UPGRADE_SERVICE_IMPL_API
#  elif defined(UPGRADE_SERVICE_IMPL_SHARED)
#    define UPGRADE_SERVICE_IMPL_API __declspec(dllexport)
#  else
#    define UPGRADE_SERVICE_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(UPGRADE_SERVICE_IMPL_STATIC)
#    define UPGRADE_SERVICE_IMPL_API
#  else
#    define UPGRADE_SERVICE_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
