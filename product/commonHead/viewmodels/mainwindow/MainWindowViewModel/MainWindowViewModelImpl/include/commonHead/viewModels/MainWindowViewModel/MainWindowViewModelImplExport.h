#pragma once

#if defined(MAIN_WINDOW_VIEW_MODEL_IMPL_STATIC) && defined(MAIN_WINDOW_VIEW_MODEL_IMPL_SHARED)
#  error "MAIN_WINDOW_VIEW_MODEL_IMPL_STATIC and MAIN_WINDOW_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(MAIN_WINDOW_VIEW_MODEL_IMPL_STATIC)
#    define MAIN_WINDOW_VIEW_MODEL_IMPL_API
#  elif defined(MAIN_WINDOW_VIEW_MODEL_IMPL_SHARED)
#    define MAIN_WINDOW_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define MAIN_WINDOW_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(MAIN_WINDOW_VIEW_MODEL_IMPL_STATIC)
#    define MAIN_WINDOW_VIEW_MODEL_IMPL_API
#  else
#    define MAIN_WINDOW_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
