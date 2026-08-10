#pragma once

// Per-module export macro for ScreenRecordingAgent.
//   SCREEN_RECORDING_AGENT_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   SCREEN_RECORDING_AGENT_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(SCREEN_RECORDING_AGENT_STATIC) && defined(SCREEN_RECORDING_AGENT_SHARED)
#  error "SCREEN_RECORDING_AGENT_STATIC and SCREEN_RECORDING_AGENT_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(SCREEN_RECORDING_AGENT_STATIC)
#    define SCREEN_RECORDING_AGENT_API
#  elif defined(SCREEN_RECORDING_AGENT_SHARED)
#    define SCREEN_RECORDING_AGENT_API __declspec(dllexport)
#  else
#    define SCREEN_RECORDING_AGENT_API __declspec(dllimport)
#  endif
#else
#  if defined(SCREEN_RECORDING_AGENT_STATIC)
#    define SCREEN_RECORDING_AGENT_API
#  else
#    define SCREEN_RECORDING_AGENT_API __attribute__((visibility("default")))
#  endif
#endif
