#pragma once

#if defined(SERVICE_LOCATOR_IMPL_STATIC) && defined(SERVICE_LOCATOR_IMPL_SHARED)
#  error "SERVICE_LOCATOR_IMPL_STATIC and SERVICE_LOCATOR_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(SERVICE_LOCATOR_IMPL_STATIC)
#    define SERVICE_LOCATOR_IMPL_API
#  elif defined(SERVICE_LOCATOR_IMPL_SHARED)
#    define SERVICE_LOCATOR_IMPL_API __declspec(dllexport)
#  else
#    define SERVICE_LOCATOR_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(SERVICE_LOCATOR_IMPL_STATIC)
#    define SERVICE_LOCATOR_IMPL_API
#  else
#    define SERVICE_LOCATOR_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
