#pragma once

#if defined(PERFORMANCE_SERVICE_IMPL_STATIC) && defined(PERFORMANCE_SERVICE_IMPL_SHARED)
#  error "PERFORMANCE_SERVICE_IMPL_STATIC and PERFORMANCE_SERVICE_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(PERFORMANCE_SERVICE_IMPL_STATIC)
#    define PERFORMANCE_SERVICE_IMPL_API
#  elif defined(PERFORMANCE_SERVICE_IMPL_SHARED)
#    define PERFORMANCE_SERVICE_IMPL_API __declspec(dllexport)
#  else
#    define PERFORMANCE_SERVICE_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(PERFORMANCE_SERVICE_IMPL_STATIC)
#    define PERFORMANCE_SERVICE_IMPL_API
#  else
#    define PERFORMANCE_SERVICE_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
