#pragma once

#if defined(APP_UI_VIEW_MODEL_IMPL_STATIC) && defined(APP_UI_VIEW_MODEL_IMPL_SHARED)
#  error "APP_UI_VIEW_MODEL_IMPL_STATIC and APP_UI_VIEW_MODEL_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(APP_UI_VIEW_MODEL_IMPL_STATIC)
#    define APP_UI_VIEW_MODEL_IMPL_API
#  elif defined(APP_UI_VIEW_MODEL_IMPL_SHARED)
#    define APP_UI_VIEW_MODEL_IMPL_API __declspec(dllexport)
#  else
#    define APP_UI_VIEW_MODEL_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(APP_UI_VIEW_MODEL_IMPL_STATIC)
#    define APP_UI_VIEW_MODEL_IMPL_API
#  else
#    define APP_UI_VIEW_MODEL_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
