#pragma once

// Per-module export macro for FilePathUtils.
//   FILE_PATH_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   FILE_PATH_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(FILE_PATH_UTILS_STATIC) && defined(FILE_PATH_UTILS_SHARED)
#  error "FILE_PATH_UTILS_STATIC and FILE_PATH_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(FILE_PATH_UTILS_STATIC)
#    define FILE_PATH_UTILS_API
#  elif defined(FILE_PATH_UTILS_SHARED)
#    define FILE_PATH_UTILS_API __declspec(dllexport)
#  else
#    define FILE_PATH_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(FILE_PATH_UTILS_STATIC)
#    define FILE_PATH_UTILS_API
#  else
#    define FILE_PATH_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
