#pragma once

// Per-module export macro for ServiceDeclaration.
//   SERVICE_DECLARATION_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   SERVICE_DECLARATION_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(SERVICE_DECLARATION_STATIC) && defined(SERVICE_DECLARATION_SHARED)
#  error "SERVICE_DECLARATION_STATIC and SERVICE_DECLARATION_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(SERVICE_DECLARATION_STATIC)
#    define SERVICE_DECLARATION_API
#  elif defined(SERVICE_DECLARATION_SHARED)
#    define SERVICE_DECLARATION_API __declspec(dllexport)
#  else
#    define SERVICE_DECLARATION_API __declspec(dllimport)
#  endif
#else
#  if defined(SERVICE_DECLARATION_STATIC)
#    define SERVICE_DECLARATION_API
#  else
#    define SERVICE_DECLARATION_API __attribute__((visibility("default")))
#  endif
#endif
