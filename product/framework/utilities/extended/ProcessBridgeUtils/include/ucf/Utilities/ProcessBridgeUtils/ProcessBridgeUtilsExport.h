#pragma once

// Per-module export macro for ProcessBridgeUtils.
//   PROCESS_BRIDGE_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   PROCESS_BRIDGE_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(PROCESS_BRIDGE_UTILS_STATIC) && defined(PROCESS_BRIDGE_UTILS_SHARED)
#  error "PROCESS_BRIDGE_UTILS_STATIC and PROCESS_BRIDGE_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(PROCESS_BRIDGE_UTILS_STATIC)
#    define PROCESS_BRIDGE_UTILS_API
#  elif defined(PROCESS_BRIDGE_UTILS_SHARED)
#    define PROCESS_BRIDGE_UTILS_API __declspec(dllexport)
#  else
#    define PROCESS_BRIDGE_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(PROCESS_BRIDGE_UTILS_STATIC)
#    define PROCESS_BRIDGE_UTILS_API
#  else
#    define PROCESS_BRIDGE_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
