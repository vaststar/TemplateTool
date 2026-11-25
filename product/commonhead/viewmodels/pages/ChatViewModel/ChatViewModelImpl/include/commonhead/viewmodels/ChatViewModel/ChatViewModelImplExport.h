#pragma once

#if defined(CHAT_VIEW_MODEL_IMPL_STATIC) && defined(CHAT_VIEW_MODEL_IMPL_SHARED)
#  error "CHAT_VIEW_MODEL_IMPL_STATIC and CHAT_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(CHAT_VIEW_MODEL_IMPL_STATIC)
#    define CHAT_VIEW_MODEL_IMPL_API
#  elif defined(CHAT_VIEW_MODEL_IMPL_SHARED)
#    define CHAT_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define CHAT_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(CHAT_VIEW_MODEL_IMPL_STATIC)
#    define CHAT_VIEW_MODEL_IMPL_API
#  else
#    define CHAT_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
