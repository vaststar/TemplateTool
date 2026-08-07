#pragma once

// Per-module export macro for ServiceFactory.
//   SERVICE_FACTORY_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   SERVICE_FACTORY_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(SERVICE_FACTORY_STATIC) && defined(SERVICE_FACTORY_SHARED)
#  error "SERVICE_FACTORY_STATIC and SERVICE_FACTORY_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(SERVICE_FACTORY_STATIC)
#    define SERVICE_FACTORY_API
#  elif defined(SERVICE_FACTORY_SHARED)
#    define SERVICE_FACTORY_API __declspec(dllexport)
#  else
#    define SERVICE_FACTORY_API __declspec(dllimport)
#  endif
#else
#  if defined(SERVICE_FACTORY_STATIC)
#    define SERVICE_FACTORY_API
#  else
#    define SERVICE_FACTORY_API __attribute__((visibility("default")))
#  endif
#endif
