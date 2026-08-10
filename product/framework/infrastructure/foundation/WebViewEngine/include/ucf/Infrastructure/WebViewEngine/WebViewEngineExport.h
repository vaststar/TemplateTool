#pragma once

// Per-module export macro for WebViewEngine.
//   WEB_VIEW_ENGINE_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   WEB_VIEW_ENGINE_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(WEB_VIEW_ENGINE_STATIC) && defined(WEB_VIEW_ENGINE_SHARED)
#  error "WEB_VIEW_ENGINE_STATIC and WEB_VIEW_ENGINE_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(WEB_VIEW_ENGINE_STATIC)
#    define WEB_VIEW_ENGINE_API
#  elif defined(WEB_VIEW_ENGINE_SHARED)
#    define WEB_VIEW_ENGINE_API __declspec(dllexport)
#  else
#    define WEB_VIEW_ENGINE_API __declspec(dllimport)
#  endif
#else
#  if defined(WEB_VIEW_ENGINE_STATIC)
#    define WEB_VIEW_ENGINE_API
#  else
#    define WEB_VIEW_ENGINE_API __attribute__((visibility("default")))
#  endif
#endif
