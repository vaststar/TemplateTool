#pragma once

// Per-module export macro for DataWarehouseServiceTypes.
//   DATA_WAREHOUSE_SERVICE_TYPES_SHARED : defined PRIVATE when built as a shared lib -> dllexport; undefined in consumers -> dllimport.
//   DATA_WAREHOUSE_SERVICE_TYPES_STATIC : defined PUBLIC when built as a static lib -> empty.
#if defined(DATA_WAREHOUSE_SERVICE_TYPES_STATIC) && defined(DATA_WAREHOUSE_SERVICE_TYPES_SHARED)
#  error "DATA_WAREHOUSE_SERVICE_TYPES_STATIC and DATA_WAREHOUSE_SERVICE_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(DATA_WAREHOUSE_SERVICE_TYPES_STATIC)
#    define DATA_WAREHOUSE_SERVICE_TYPES_API
#  elif defined(DATA_WAREHOUSE_SERVICE_TYPES_SHARED)
#    define DATA_WAREHOUSE_SERVICE_TYPES_API __declspec(dllexport)
#  else
#    define DATA_WAREHOUSE_SERVICE_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(DATA_WAREHOUSE_SERVICE_TYPES_STATIC)
#    define DATA_WAREHOUSE_SERVICE_TYPES_API
#  else
#    define DATA_WAREHOUSE_SERVICE_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
