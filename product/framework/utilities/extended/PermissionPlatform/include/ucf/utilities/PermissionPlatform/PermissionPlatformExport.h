#pragma once

// PERMISSION_PLATFORM_STATIC is public for static builds. PERMISSION_PLATFORM_SHARED
// is private to this module when exporting a shared library.
#if defined(PERMISSION_PLATFORM_STATIC) && defined(PERMISSION_PLATFORM_SHARED)
#  error "PERMISSION_PLATFORM_STATIC and PERMISSION_PLATFORM_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(PERMISSION_PLATFORM_STATIC)
#    define PERMISSION_PLATFORM_API
#  elif defined(PERMISSION_PLATFORM_SHARED)
#    define PERMISSION_PLATFORM_API __declspec(dllexport)
#  else
#    define PERMISSION_PLATFORM_API __declspec(dllimport)
#  endif
#else
#  if defined(PERMISSION_PLATFORM_STATIC)
#    define PERMISSION_PLATFORM_API
#  else
#    define PERMISSION_PLATFORM_API __attribute__((visibility("default")))
#  endif
#endif
