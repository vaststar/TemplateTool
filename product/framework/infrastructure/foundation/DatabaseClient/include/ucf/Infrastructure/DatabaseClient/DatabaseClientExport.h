#pragma once

// Per-module export macro for DatabaseClient.
//   DATABASE_CLIENT_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   DATABASE_CLIENT_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(DATABASE_CLIENT_STATIC) && defined(DATABASE_CLIENT_SHARED)
#  error "DATABASE_CLIENT_STATIC and DATABASE_CLIENT_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(DATABASE_CLIENT_STATIC)
#    define DATABASE_CLIENT_API
#  elif defined(DATABASE_CLIENT_SHARED)
#    define DATABASE_CLIENT_API __declspec(dllexport)
#  else
#    define DATABASE_CLIENT_API __declspec(dllimport)
#  endif
#else
#  if defined(DATABASE_CLIENT_STATIC)
#    define DATABASE_CLIENT_API
#  else
#    define DATABASE_CLIENT_API __attribute__((visibility("default")))
#  endif
#endif
