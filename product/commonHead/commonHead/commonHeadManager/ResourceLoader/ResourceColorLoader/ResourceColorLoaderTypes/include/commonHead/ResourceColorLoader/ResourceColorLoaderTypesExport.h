#pragma once

#if defined(RESOURCE_COLOR_LOADER_TYPES_STATIC) && defined(RESOURCE_COLOR_LOADER_TYPES_SHARED)
#  error "RESOURCE_COLOR_LOADER_TYPES_STATIC and RESOURCE_COLOR_LOADER_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(RESOURCE_COLOR_LOADER_TYPES_STATIC)
#    define RESOURCE_COLOR_LOADER_TYPES_API
#  elif defined(RESOURCE_COLOR_LOADER_TYPES_SHARED)
#    define RESOURCE_COLOR_LOADER_TYPES_API __declspec(dllexport)
#  else
#    define RESOURCE_COLOR_LOADER_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(RESOURCE_COLOR_LOADER_TYPES_STATIC)
#    define RESOURCE_COLOR_LOADER_TYPES_API
#  else
#    define RESOURCE_COLOR_LOADER_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
