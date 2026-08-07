#pragma once

// Per-module export macro for NetworkServiceImpl.
//   NETWORK_SERVICE_IMPL_SHARED : 动态构建本模块时由 CMake 以 PRIVATE 传入 -> dllexport；
//                                 消费者未定义它 -> dllimport。
//   NETWORK_SERVICE_IMPL_STATIC : 静态构建时由 CMake 以 PUBLIC 传入（传播给消费者）-> 空。
#if defined(NETWORK_SERVICE_IMPL_STATIC) && defined(NETWORK_SERVICE_IMPL_SHARED)
#  error "NETWORK_SERVICE_IMPL_STATIC and NETWORK_SERVICE_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(NETWORK_SERVICE_IMPL_STATIC)
#    define NETWORK_SERVICE_IMPL_API
#  elif defined(NETWORK_SERVICE_IMPL_SHARED)
#    define NETWORK_SERVICE_IMPL_API __declspec(dllexport)
#  else
#    define NETWORK_SERVICE_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(NETWORK_SERVICE_IMPL_STATIC)
#    define NETWORK_SERVICE_IMPL_API
#  else
#    define NETWORK_SERVICE_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
