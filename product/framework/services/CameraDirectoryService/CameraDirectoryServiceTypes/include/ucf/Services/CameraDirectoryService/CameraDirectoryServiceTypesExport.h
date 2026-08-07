#pragma once

// Per-module export macro for CameraDirectoryServiceTypes.
//   CAMERA_DIRECTORY_SERVICE_TYPES_SHARED : 动态构建本模块时由 CMake 以 PRIVATE 传入 -> dllexport；
//                                           消费者未定义它 -> dllimport。
//   CAMERA_DIRECTORY_SERVICE_TYPES_STATIC : 静态构建时由 CMake 以 PUBLIC 传入（传播给消费者）-> 空。
#if defined(CAMERA_DIRECTORY_SERVICE_TYPES_STATIC) && defined(CAMERA_DIRECTORY_SERVICE_TYPES_SHARED)
#  error "CAMERA_DIRECTORY_SERVICE_TYPES_STATIC and CAMERA_DIRECTORY_SERVICE_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(CAMERA_DIRECTORY_SERVICE_TYPES_STATIC)
#    define CAMERA_DIRECTORY_SERVICE_TYPES_API
#  elif defined(CAMERA_DIRECTORY_SERVICE_TYPES_SHARED)
#    define CAMERA_DIRECTORY_SERVICE_TYPES_API __declspec(dllexport)
#  else
#    define CAMERA_DIRECTORY_SERVICE_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(CAMERA_DIRECTORY_SERVICE_TYPES_STATIC)
#    define CAMERA_DIRECTORY_SERVICE_TYPES_API
#  else
#    define CAMERA_DIRECTORY_SERVICE_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
