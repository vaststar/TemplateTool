#pragma once

#if defined(CONTACT_LIST_VIEW_MODEL_IMPL_STATIC) && defined(CONTACT_LIST_VIEW_MODEL_IMPL_SHARED)
#  error "CONTACT_LIST_VIEW_MODEL_IMPL_STATIC and CONTACT_LIST_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(CONTACT_LIST_VIEW_MODEL_IMPL_STATIC)
#    define CONTACT_LIST_VIEW_MODEL_IMPL_API
#  elif defined(CONTACT_LIST_VIEW_MODEL_IMPL_SHARED)
#    define CONTACT_LIST_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define CONTACT_LIST_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(CONTACT_LIST_VIEW_MODEL_IMPL_STATIC)
#    define CONTACT_LIST_VIEW_MODEL_IMPL_API
#  else
#    define CONTACT_LIST_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
