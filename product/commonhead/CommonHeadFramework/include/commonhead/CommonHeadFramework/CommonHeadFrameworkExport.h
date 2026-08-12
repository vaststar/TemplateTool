#pragma once

// Per-module export macro for CommonHeadFramework.
//   COMMON_HEAD_FRAMEWORK_SHARED: defined PRIVATE when building the shared library.
//   COMMON_HEAD_FRAMEWORK_STATIC: defined PUBLIC when building the static library.
#if defined(COMMON_HEAD_FRAMEWORK_STATIC) && defined(COMMON_HEAD_FRAMEWORK_SHARED)
#  error "COMMON_HEAD_FRAMEWORK_STATIC and COMMON_HEAD_FRAMEWORK_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(COMMON_HEAD_FRAMEWORK_STATIC)
#    define COMMON_HEAD_FRAMEWORK_API
#  elif defined(COMMON_HEAD_FRAMEWORK_SHARED)
#    define COMMON_HEAD_FRAMEWORK_API __declspec(dllexport)
#  else
#    define COMMON_HEAD_FRAMEWORK_API __declspec(dllimport)
#  endif
#else
#  if defined(COMMON_HEAD_FRAMEWORK_STATIC)
#    define COMMON_HEAD_FRAMEWORK_API
#  else
#    define COMMON_HEAD_FRAMEWORK_API __attribute__((visibility("default")))
#  endif
#endif
