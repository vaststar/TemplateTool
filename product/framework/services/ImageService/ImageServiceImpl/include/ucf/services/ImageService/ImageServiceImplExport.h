#pragma once

#if defined(IMAGE_SERVICE_IMPL_STATIC) && defined(IMAGE_SERVICE_IMPL_SHARED)
#  error "IMAGE_SERVICE_IMPL_STATIC and IMAGE_SERVICE_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(IMAGE_SERVICE_IMPL_STATIC)
#    define IMAGE_SERVICE_IMPL_API
#  elif defined(IMAGE_SERVICE_IMPL_SHARED)
#    define IMAGE_SERVICE_IMPL_API __declspec(dllexport)
#  else
#    define IMAGE_SERVICE_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(IMAGE_SERVICE_IMPL_STATIC)
#    define IMAGE_SERVICE_IMPL_API
#  else
#    define IMAGE_SERVICE_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
