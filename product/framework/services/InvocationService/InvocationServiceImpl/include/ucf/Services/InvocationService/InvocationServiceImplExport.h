#pragma once

// Per-module export macro for InvocationServiceImpl.
//   INVOCATION_SERVICE_IMPL_SHARED : 动态构建本模块时由 CMake 以 PRIVATE 传入 -> dllexport；
//                                    消费者未定义它 -> dllimport。
//   INVOCATION_SERVICE_IMPL_STATIC : 静态构建时由 CMake 以 PUBLIC 传入（传播给消费者）-> 空。
#if defined(INVOCATION_SERVICE_IMPL_STATIC) && defined(INVOCATION_SERVICE_IMPL_SHARED)
#  error "INVOCATION_SERVICE_IMPL_STATIC and INVOCATION_SERVICE_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(INVOCATION_SERVICE_IMPL_STATIC)
#    define INVOCATION_SERVICE_IMPL_API
#  elif defined(INVOCATION_SERVICE_IMPL_SHARED)
#    define INVOCATION_SERVICE_IMPL_API __declspec(dllexport)
#  else
#    define INVOCATION_SERVICE_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(INVOCATION_SERVICE_IMPL_STATIC)
#    define INVOCATION_SERVICE_IMPL_API
#  else
#    define INVOCATION_SERVICE_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
