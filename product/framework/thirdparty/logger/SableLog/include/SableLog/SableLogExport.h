#pragma once

#if defined(SABLELOG_STATIC) && defined(SABLELOG_SHARED)
#  error "SABLELOG_STATIC and SABLELOG_SHARED cannot be defined together"
#endif

#if defined(SABLELOG_STATIC)
#  define SABLELOG_API
#elif defined(_WIN32)
#  if defined(SABLELOG_SHARED)
#    define SABLELOG_API __declspec(dllexport)
#  else
#    define SABLELOG_API __declspec(dllimport)
#  endif
#elif defined(__GNUC__) || defined(__clang__)
#  define SABLELOG_API __attribute__((visibility("default")))
#else
#  define SABLELOG_API
#endif
