#pragma once

// Per-module export macro for MiniAppRuntimeAgent.
//   MINI_APP_RUNTIME_AGENT_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   MINI_APP_RUNTIME_AGENT_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(MINI_APP_RUNTIME_AGENT_STATIC) && defined(MINI_APP_RUNTIME_AGENT_SHARED)
#  error "MINI_APP_RUNTIME_AGENT_STATIC and MINI_APP_RUNTIME_AGENT_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(MINI_APP_RUNTIME_AGENT_STATIC)
#    define MINI_APP_RUNTIME_AGENT_API
#  elif defined(MINI_APP_RUNTIME_AGENT_SHARED)
#    define MINI_APP_RUNTIME_AGENT_API __declspec(dllexport)
#  else
#    define MINI_APP_RUNTIME_AGENT_API __declspec(dllimport)
#  endif
#else
#  if defined(MINI_APP_RUNTIME_AGENT_STATIC)
#    define MINI_APP_RUNTIME_AGENT_API
#  else
#    define MINI_APP_RUNTIME_AGENT_API __attribute__((visibility("default")))
#  endif
#endif
