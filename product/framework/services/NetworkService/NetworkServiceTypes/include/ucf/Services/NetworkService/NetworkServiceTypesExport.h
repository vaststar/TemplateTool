#pragma once

// Per-module export macro for NetworkServiceTypes.
//   NETWORK_SERVICE_TYPES_SHARED : 动态构建本模块时由 CMake 以 PRIVATE 传入 -> dllexport；
//                                  消费者未定义它 -> dllimport。
//   NETWORK_SERVICE_TYPES_STATIC : 静态构建时由 CMake 以 PUBLIC 传入（传播给消费者）-> 空。
#if defined(NETWORK_SERVICE_TYPES_STATIC) && defined(NETWORK_SERVICE_TYPES_SHARED)
#  error "NETWORK_SERVICE_TYPES_STATIC and NETWORK_SERVICE_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(NETWORK_SERVICE_TYPES_STATIC)
#    define NETWORK_SERVICE_TYPES_API
#  elif defined(NETWORK_SERVICE_TYPES_SHARED)
#    define NETWORK_SERVICE_TYPES_API __declspec(dllexport)
#  else
#    define NETWORK_SERVICE_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(NETWORK_SERVICE_TYPES_STATIC)
#    define NETWORK_SERVICE_TYPES_API
#  else
#    define NETWORK_SERVICE_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
