#pragma once

// Per-module export macro for CoreFramework.
//   CORE_FRAMEWORK_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   CORE_FRAMEWORK_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(CORE_FRAMEWORK_STATIC) && defined(CORE_FRAMEWORK_SHARED)
#  error "CORE_FRAMEWORK_STATIC and CORE_FRAMEWORK_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(CORE_FRAMEWORK_STATIC)
#    define CORE_FRAMEWORK_API
#  elif defined(CORE_FRAMEWORK_SHARED)
#    define CORE_FRAMEWORK_API __declspec(dllexport)
#  else
#    define CORE_FRAMEWORK_API __declspec(dllimport)
#  endif
#else
#  if defined(CORE_FRAMEWORK_STATIC)
#    define CORE_FRAMEWORK_API
#  else
#    define CORE_FRAMEWORK_API __attribute__((visibility("default")))
#  endif
#endif
