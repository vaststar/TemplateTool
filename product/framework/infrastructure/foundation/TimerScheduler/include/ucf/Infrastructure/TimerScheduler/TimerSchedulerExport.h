#pragma once

// Per-module export macro for TimerScheduler.
//   TIMER_SCHEDULER_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   TIMER_SCHEDULER_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(TIMER_SCHEDULER_STATIC) && defined(TIMER_SCHEDULER_SHARED)
#  error "TIMER_SCHEDULER_STATIC and TIMER_SCHEDULER_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(TIMER_SCHEDULER_STATIC)
#    define TIMER_SCHEDULER_API
#  elif defined(TIMER_SCHEDULER_SHARED)
#    define TIMER_SCHEDULER_API __declspec(dllexport)
#  else
#    define TIMER_SCHEDULER_API __declspec(dllimport)
#  endif
#else
#  if defined(TIMER_SCHEDULER_STATIC)
#    define TIMER_SCHEDULER_API
#  else
#    define TIMER_SCHEDULER_API __attribute__((visibility("default")))
#  endif
#endif
