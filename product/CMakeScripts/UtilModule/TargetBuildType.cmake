# Check if a target is a shared (dynamic) library.
# Usage:
#   target_is_shared_library(MyTarget result)
#   -> result = TRUE  if MyTarget is SHARED_LIBRARY
#   -> result = FALSE if MyTarget is STATIC_LIBRARY or others
include_guard()
function(target_is_shared_library target_name result_var)
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "target_is_shared_library: '${target_name}' is not a known CMake target.")
    endif()

    get_target_property(_type ${target_name} TYPE)

    if(NOT _type)
        message(FATAL_ERROR "target_is_shared_library: Failed to get target type for '${target_name}'.")
    endif()

    if(_type STREQUAL "SHARED_LIBRARY")
        set(${result_var} TRUE PARENT_SCOPE)
    elseif(_type STREQUAL "STATIC_LIBRARY")
        set(${result_var} FALSE PARENT_SCOPE)
    else()
        message(WARNING "target_is_shared_library: Target '${target_name}' is of type '${_type}', which is not a library. Returning FALSE.")
        set(${result_var} FALSE PARENT_SCOPE)
    endif()
endfunction()
