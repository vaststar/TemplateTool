#pragma once

#if defined(JSON_TREE_VIEW_MODEL_TYPES_STATIC) && defined(JSON_TREE_VIEW_MODEL_TYPES_SHARED)
#  error "JSON_TREE_VIEW_MODEL_TYPES_STATIC and JSON_TREE_VIEW_MODEL_TYPES_SHARED cannot be defined together"
#endif

#if defined(_WIN32)
#  pragma warning(disable : 4251)
#  if defined(JSON_TREE_VIEW_MODEL_TYPES_STATIC)
#    define JSON_TREE_VIEW_MODEL_TYPES_API
#  elif defined(JSON_TREE_VIEW_MODEL_TYPES_SHARED)
#    define JSON_TREE_VIEW_MODEL_TYPES_API __declspec(dllexport)
#  else
#    define JSON_TREE_VIEW_MODEL_TYPES_API __declspec(dllimport)
#  endif
#else
#  if defined(JSON_TREE_VIEW_MODEL_TYPES_STATIC)
#    define JSON_TREE_VIEW_MODEL_TYPES_API
#  else
#    define JSON_TREE_VIEW_MODEL_TYPES_API __attribute__((visibility("default")))
#  endif
#endif
