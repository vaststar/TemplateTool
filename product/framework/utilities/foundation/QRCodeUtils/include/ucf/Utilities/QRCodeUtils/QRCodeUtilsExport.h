#pragma once

// Per-module export macro for QRCodeUtils.
//   QRCODE_UTILS_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   QRCODE_UTILS_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(QRCODE_UTILS_STATIC) && defined(QRCODE_UTILS_SHARED)
#  error "QRCODE_UTILS_STATIC and QRCODE_UTILS_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(QRCODE_UTILS_STATIC)
#    define QRCODE_UTILS_API
#  elif defined(QRCODE_UTILS_SHARED)
#    define QRCODE_UTILS_API __declspec(dllexport)
#  else
#    define QRCODE_UTILS_API __declspec(dllimport)
#  endif
#else
#  if defined(QRCODE_UTILS_STATIC)
#    define QRCODE_UTILS_API
#  else
#    define QRCODE_UTILS_API __attribute__((visibility("default")))
#  endif
#endif
