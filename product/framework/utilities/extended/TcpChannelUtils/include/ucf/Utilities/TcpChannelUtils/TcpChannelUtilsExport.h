#pragma once

// Per-module export macro for TcpChannelUtils.
//   TCP_CHANNEL_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   TCP_CHANNEL_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(TCP_CHANNEL_UTILS_STATIC) && defined(TCP_CHANNEL_UTILS_SHARED)
#  error "TCP_CHANNEL_UTILS_STATIC and TCP_CHANNEL_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(TCP_CHANNEL_UTILS_STATIC)
#    define TCP_CHANNEL_UTILS_API
#  elif defined(TCP_CHANNEL_UTILS_SHARED)
#    define TCP_CHANNEL_UTILS_API __declspec(dllexport)
#  else
#    define TCP_CHANNEL_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(TCP_CHANNEL_UTILS_STATIC)
#    define TCP_CHANNEL_UTILS_API
#  else
#    define TCP_CHANNEL_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
