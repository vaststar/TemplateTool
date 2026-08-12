#pragma once

// Per-module export macro for ArchiveUtils.
//   ARCHIVE_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   ARCHIVE_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(ARCHIVE_UTILS_STATIC) && defined(ARCHIVE_UTILS_SHARED)
#  error "ARCHIVE_UTILS_STATIC and ARCHIVE_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(ARCHIVE_UTILS_STATIC)
#    define ARCHIVE_UTILS_API
#  elif defined(ARCHIVE_UTILS_SHARED)
#    define ARCHIVE_UTILS_API __declspec(dllexport)
#  else
#    define ARCHIVE_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(ARCHIVE_UTILS_STATIC)
#    define ARCHIVE_UTILS_API
#  else
#    define ARCHIVE_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
