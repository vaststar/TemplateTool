#pragma once

// Per-module export macro for StringUtils.
//   STRING_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   STRING_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(STRING_UTILS_STATIC) && defined(STRING_UTILS_SHARED)
#  error "STRING_UTILS_STATIC and STRING_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(STRING_UTILS_STATIC)
#    define STRING_UTILS_API
#  elif defined(STRING_UTILS_SHARED)
#    define STRING_UTILS_API __declspec(dllexport)
#  else
#    define STRING_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(STRING_UTILS_STATIC)
#    define STRING_UTILS_API
#  else
#    define STRING_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
