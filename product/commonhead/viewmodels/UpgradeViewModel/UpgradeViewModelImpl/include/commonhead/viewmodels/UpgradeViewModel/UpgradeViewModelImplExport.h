#pragma once

// Per-module export macro for UpgradeViewModelImpl.
//   UPGRADE_VIEW_MODEL_IMPL_SHARED: defined PRIVATE when building the shared library.
//   UPGRADE_VIEW_MODEL_IMPL_STATIC: defined PUBLIC when building the static library.
#if defined(UPGRADE_VIEW_MODEL_IMPL_STATIC) && defined(UPGRADE_VIEW_MODEL_IMPL_SHARED)
#  error "UPGRADE_VIEW_MODEL_IMPL_STATIC and UPGRADE_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(UPGRADE_VIEW_MODEL_IMPL_STATIC)
#    define UPGRADE_VIEW_MODEL_IMPL_API
#  elif defined(UPGRADE_VIEW_MODEL_IMPL_SHARED)
#    define UPGRADE_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define UPGRADE_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(UPGRADE_VIEW_MODEL_IMPL_STATIC)
#    define UPGRADE_VIEW_MODEL_IMPL_API
#  else
#    define UPGRADE_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
