#pragma once

#if defined(CHAT_VIEW_MODEL_TYPES_STATIC) && defined(CHAT_VIEW_MODEL_TYPES_SHARED)
#  error "CHAT_VIEW_MODEL_TYPES_STATIC and CHAT_VIEW_MODEL_TYPES_SHARED cannot be defined together"
#endif

#if defined(CHAT_VIEW_MODEL_TYPES_STATIC)
#  define CHAT_VIEW_MODEL_TYPES_API
#elif defined(_WIN32)
#  if defined(CHAT_VIEW_MODEL_TYPES_SHARED)
#    define CHAT_VIEW_MODEL_TYPES_API __declspec(dllexport)
#  else
#    define CHAT_VIEW_MODEL_TYPES_API __declspec(dllimport)
#  endif
#else
#  define CHAT_VIEW_MODEL_TYPES_API __attribute__((visibility("default")))
#endif
