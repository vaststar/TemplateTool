#pragma once

// Per-module export macro for InvocationServiceImpl.
//   INVOCATION_SERVICE_IMPL_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   INVOCATION_SERVICE_IMPL_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(INVOCATION_SERVICE_IMPL_STATIC) && defined(INVOCATION_SERVICE_IMPL_SHARED)
#  error "INVOCATION_SERVICE_IMPL_STATIC and INVOCATION_SERVICE_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(INVOCATION_SERVICE_IMPL_STATIC)
#    define INVOCATION_SERVICE_IMPL_API
#  elif defined(INVOCATION_SERVICE_IMPL_SHARED)
#    define INVOCATION_SERVICE_IMPL_API __declspec(dllexport)
#  else
#    define INVOCATION_SERVICE_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(INVOCATION_SERVICE_IMPL_STATIC)
#    define INVOCATION_SERVICE_IMPL_API
#  else
#    define INVOCATION_SERVICE_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
