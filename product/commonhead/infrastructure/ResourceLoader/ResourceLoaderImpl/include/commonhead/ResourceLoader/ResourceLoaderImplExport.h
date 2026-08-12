#pragma once

#if defined(RESOURCE_LOADER_IMPL_STATIC) && defined(RESOURCE_LOADER_IMPL_SHARED)
#  error "RESOURCE_LOADER_IMPL_STATIC and RESOURCE_LOADER_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(RESOURCE_LOADER_IMPL_STATIC)
#    define RESOURCE_LOADER_IMPL_API
#  elif defined(RESOURCE_LOADER_IMPL_SHARED)
#    define RESOURCE_LOADER_IMPL_API __declspec(dllexport)
#  else
#    define RESOURCE_LOADER_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(RESOURCE_LOADER_IMPL_STATIC)
#    define RESOURCE_LOADER_IMPL_API
#  else
#    define RESOURCE_LOADER_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
