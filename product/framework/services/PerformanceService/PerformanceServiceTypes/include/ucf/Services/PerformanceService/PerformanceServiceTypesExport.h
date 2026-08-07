#pragma once

// Per-module export macro for PerformanceServiceTypes.
//   PERFORMANCE_SERVICE_TYPES_SHARED : 动态构建本模块时由 CMake 以 PRIVATE 传入 -> dllexport；
//                                消费者未定义它 -> dllimport。
//   PERFORMANCE_SERVICE_TYPES_STATIC : 静态构建时由 CMake 以 PUBLIC 传入（传播给消费者）-> 空。
#if defined(PERFORMANCE_SERVICE_TYPES_STATIC) && defined(PERFORMANCE_SERVICE_TYPES_SHARED)
#  error "PERFORMANCE_SERVICE_TYPES_STATIC and PERFORMANCE_SERVICE_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(PERFORMANCE_SERVICE_TYPES_STATIC)
#    define PERFORMANCE_SERVICE_TYPES_API
#  elif defined(PERFORMANCE_SERVICE_TYPES_SHARED)
#    define PERFORMANCE_SERVICE_TYPES_API __declspec(dllexport)
#  else
#    define PERFORMANCE_SERVICE_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(PERFORMANCE_SERVICE_TYPES_STATIC)
#    define PERFORMANCE_SERVICE_TYPES_API
#  else
#    define PERFORMANCE_SERVICE_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
