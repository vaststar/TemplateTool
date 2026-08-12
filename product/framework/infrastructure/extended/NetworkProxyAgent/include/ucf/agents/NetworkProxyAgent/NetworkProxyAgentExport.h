#pragma once

// Per-module export macro for NetworkProxyAgent.
//   NETWORK_PROXY_AGENT_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   NETWORK_PROXY_AGENT_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(NETWORK_PROXY_AGENT_STATIC) && defined(NETWORK_PROXY_AGENT_SHARED)
#  error "NETWORK_PROXY_AGENT_STATIC and NETWORK_PROXY_AGENT_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(NETWORK_PROXY_AGENT_STATIC)
#    define NETWORK_PROXY_AGENT_API
#  elif defined(NETWORK_PROXY_AGENT_SHARED)
#    define NETWORK_PROXY_AGENT_API __declspec(dllexport)
#  else
#    define NETWORK_PROXY_AGENT_API __declspec(dllimport)
#  endif
#else
#  if defined(NETWORK_PROXY_AGENT_STATIC)
#    define NETWORK_PROXY_AGENT_API
#  else
#    define NETWORK_PROXY_AGENT_API __attribute__((visibility("default")))
#  endif
#endif
