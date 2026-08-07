#pragma once

// Per-module export macro for CoreFramework.
//   CORE_FRAMEWORK_SHARED : 动态构建本模块时由 CMake 以 PRIVATE 传入 -> dllexport；
//                           消费者未定义它 -> dllimport。
//   CORE_FRAMEWORK_STATIC : 静态构建时由 CMake 以 PUBLIC 传入（传播给消费者）-> 空。
#if defined(CORE_FRAMEWORK_STATIC) && defined(CORE_FRAMEWORK_SHARED)
#  error "CORE_FRAMEWORK_STATIC and CORE_FRAMEWORK_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(CORE_FRAMEWORK_STATIC)
#    define CORE_FRAMEWORK_API
#  elif defined(CORE_FRAMEWORK_SHARED)
#    define CORE_FRAMEWORK_API __declspec(dllexport)
#  else
#    define CORE_FRAMEWORK_API __declspec(dllimport)
#  endif
#else
#  if defined(CORE_FRAMEWORK_STATIC)
#    define CORE_FRAMEWORK_API
#  else
#    define CORE_FRAMEWORK_API __attribute__((visibility("default")))
#  endif
#endif
