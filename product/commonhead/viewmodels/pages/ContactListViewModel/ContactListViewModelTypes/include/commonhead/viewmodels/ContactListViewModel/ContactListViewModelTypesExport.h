#pragma once

#if defined(CONTACT_LIST_VIEW_MODEL_TYPES_STATIC) && defined(CONTACT_LIST_VIEW_MODEL_TYPES_SHARED)
#  error "CONTACT_LIST_VIEW_MODEL_TYPES_STATIC and CONTACT_LIST_VIEW_MODEL_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(CONTACT_LIST_VIEW_MODEL_TYPES_STATIC)
#    define CONTACT_LIST_VIEW_MODEL_TYPES_API
#  elif defined(CONTACT_LIST_VIEW_MODEL_TYPES_SHARED)
#    define CONTACT_LIST_VIEW_MODEL_TYPES_API __declspec(dllexport)
#  else
#    define CONTACT_LIST_VIEW_MODEL_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(CONTACT_LIST_VIEW_MODEL_TYPES_STATIC)
#    define CONTACT_LIST_VIEW_MODEL_TYPES_API
#  else
#    define CONTACT_LIST_VIEW_MODEL_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
