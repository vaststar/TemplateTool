#pragma once

// Per-module export macro for CoreFrameworkImpl.
//   CORE_FRAMEWORK_IMPL_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   CORE_FRAMEWORK_IMPL_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(CORE_FRAMEWORK_IMPL_STATIC) && defined(CORE_FRAMEWORK_IMPL_SHARED)
#  error "CORE_FRAMEWORK_IMPL_STATIC and CORE_FRAMEWORK_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(CORE_FRAMEWORK_IMPL_STATIC)
#    define CORE_FRAMEWORK_IMPL_API
#  elif defined(CORE_FRAMEWORK_IMPL_SHARED)
#    define CORE_FRAMEWORK_IMPL_API __declspec(dllexport)
#  else
#    define CORE_FRAMEWORK_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(CORE_FRAMEWORK_IMPL_STATIC)
#    define CORE_FRAMEWORK_IMPL_API
#  else
#    define CORE_FRAMEWORK_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
