#pragma once

// Per-module export macro for UpgradeServiceImpl.
//   UPGRADE_SERVICE_IMPL_SHARED : 动态构建本模块时由 CMake 以 PRIVATE 传入 -> dllexport；
//                                 消费者未定义它 -> dllimport。
//   UPGRADE_SERVICE_IMPL_STATIC : 静态构建时由 CMake 以 PUBLIC 传入（传播给消费者）-> 空。
#if defined(UPGRADE_SERVICE_IMPL_STATIC) && defined(UPGRADE_SERVICE_IMPL_SHARED)
#  error "UPGRADE_SERVICE_IMPL_STATIC and UPGRADE_SERVICE_IMPL_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  if defined(UPGRADE_SERVICE_IMPL_STATIC)
#    define UPGRADE_SERVICE_IMPL_API
#  elif defined(UPGRADE_SERVICE_IMPL_SHARED)
#    define UPGRADE_SERVICE_IMPL_API __declspec(dllexport)
#  else
#    define UPGRADE_SERVICE_IMPL_API __declspec(dllimport)
#  endif
#else
#  if defined(UPGRADE_SERVICE_IMPL_STATIC)
#    define UPGRADE_SERVICE_IMPL_API
#  else
#    define UPGRADE_SERVICE_IMPL_API __attribute__((visibility("default")))
#  endif
#endif
