#pragma once

#if defined(CHAT_SERVICE_IMPL_STATIC) && defined(CHAT_SERVICE_IMPL_SHARED)
#  error "CHAT_SERVICE_IMPL_STATIC and CHAT_SERVICE_IMPL_SHARED cannot be defined together"
#endif

#if defined(CHAT_SERVICE_IMPL_STATIC)
#  define CHAT_SERVICE_IMPL_API
#elif defined(_WIN32)
#  if defined(CHAT_SERVICE_IMPL_SHARED)
#    define CHAT_SERVICE_IMPL_API __declspec(dllexport)
#  else
#    define CHAT_SERVICE_IMPL_API __declspec(dllimport)
#  endif
#else
#  define CHAT_SERVICE_IMPL_API __attribute__((visibility("default")))
#endif
