#pragma once

// Per-module export macro for ViewModelCore.
//   VIEW_MODEL_CORE_SHARED: defined PRIVATE when building the shared library.
//   VIEW_MODEL_CORE_STATIC: defined PUBLIC when building the static library.
#if defined(VIEW_MODEL_CORE_STATIC) && defined(VIEW_MODEL_CORE_SHARED)
#  error "VIEW_MODEL_CORE_STATIC and VIEW_MODEL_CORE_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(VIEW_MODEL_CORE_STATIC)
#    define VIEW_MODEL_CORE_API
#  elif defined(VIEW_MODEL_CORE_SHARED)
#    define VIEW_MODEL_CORE_API __declspec(dllexport)
#  else
#    define VIEW_MODEL_CORE_API __declspec(dllimport)
#  endif
#else
#  if defined(VIEW_MODEL_CORE_STATIC)
#    define VIEW_MODEL_CORE_API
#  else
#    define VIEW_MODEL_CORE_API __attribute__((visibility("default")))
#  endif
#endif
