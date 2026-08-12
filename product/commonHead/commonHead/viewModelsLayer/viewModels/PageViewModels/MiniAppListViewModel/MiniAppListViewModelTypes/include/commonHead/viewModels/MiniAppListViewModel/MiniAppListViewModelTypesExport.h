#pragma once

#if defined(MINI_APP_LIST_VIEW_MODEL_TYPES_STATIC) && defined(MINI_APP_LIST_VIEW_MODEL_TYPES_SHARED)
#  error "MINI_APP_LIST_VIEW_MODEL_TYPES_STATIC and MINI_APP_LIST_VIEW_MODEL_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(MINI_APP_LIST_VIEW_MODEL_TYPES_STATIC)
#    define MINI_APP_LIST_VIEW_MODEL_TYPES_API
#  elif defined(MINI_APP_LIST_VIEW_MODEL_TYPES_SHARED)
#    define MINI_APP_LIST_VIEW_MODEL_TYPES_API __declspec(dllexport)
#  else
#    define MINI_APP_LIST_VIEW_MODEL_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(MINI_APP_LIST_VIEW_MODEL_TYPES_STATIC)
#    define MINI_APP_LIST_VIEW_MODEL_TYPES_API
#  else
#    define MINI_APP_LIST_VIEW_MODEL_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
