#pragma once

// Per-module export macro for DataWarehouseSchema.
//   DATA_WAREHOUSE_SCHEMA_SHARED : 动态构建本模块时由 CMake 以 PRIVATE 传入 -> dllexport；
//                                  消费者未定义它 -> dllimport。
//   DATA_WAREHOUSE_SCHEMA_STATIC : 静态构建时由 CMake 以 PUBLIC 传入（传播给消费者）-> 空。
#if defined(DATA_WAREHOUSE_SCHEMA_STATIC) && defined(DATA_WAREHOUSE_SCHEMA_SHARED)
#  error "DATA_WAREHOUSE_SCHEMA_STATIC and DATA_WAREHOUSE_SCHEMA_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(DATA_WAREHOUSE_SCHEMA_STATIC)
#    define DATA_WAREHOUSE_SCHEMA_API
#  elif defined(DATA_WAREHOUSE_SCHEMA_SHARED)
#    define DATA_WAREHOUSE_SCHEMA_API __declspec(dllexport)
#  else
#    define DATA_WAREHOUSE_SCHEMA_API __declspec(dllimport)
#  endif
#else
#  if defined(DATA_WAREHOUSE_SCHEMA_STATIC)
#    define DATA_WAREHOUSE_SCHEMA_API
#  else
#    define DATA_WAREHOUSE_SCHEMA_API __attribute__((visibility("default")))
#  endif
#endif
