#pragma once

#if defined(VIEW_MODEL_FACTORY_STATIC) && defined(VIEW_MODEL_FACTORY_SHARED)
#  error "VIEW_MODEL_FACTORY_STATIC and VIEW_MODEL_FACTORY_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(VIEW_MODEL_FACTORY_STATIC)
#    define VIEW_MODEL_FACTORY_API
#  elif defined(VIEW_MODEL_FACTORY_SHARED)
#    define VIEW_MODEL_FACTORY_API __declspec(dllexport)
#  else
#    define VIEW_MODEL_FACTORY_API __declspec(dllimport)
#  endif
#else
#  if defined(VIEW_MODEL_FACTORY_STATIC)
#    define VIEW_MODEL_FACTORY_API
#  else
#    define VIEW_MODEL_FACTORY_API __attribute__((visibility("default")))
#  endif
#endif
