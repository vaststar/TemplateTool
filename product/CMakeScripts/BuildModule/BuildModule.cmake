include_guard()

include("${CMAKE_CURRENT_LIST_DIR}/internal/TargetInternals.cmake")

# ==========================================
# Function: BuildModule
# Build a static or shared library module
# ==========================================
function(BuildModule)
    set(options STATIC_LIB SHARED_LIB NO_INSTALL)
    set(one_value_args MODULE_NAME IDE_FOLDER)
    set(multi_value_args
        TARGET_SOURCE_PRIVATE
        TARGET_SOURCE_PUBLIC_HEADER
        TARGET_SOURCE_HEADER_BASE_DIR
        TARGET_ADD_LINK_LIBRARY_PRIVATE
        TARGET_ADD_LINK_LIBRARY_PUBLIC
        TARGET_ADD_DEPENDENCIES
        TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        TARGET_INCLUDE_DIRECTORIES_PRIVATE
        TARGET_PUBLIC_DEFINITIONS
        TARGET_PRIVATE_DEFINITIONS
    )
    cmake_parse_arguments(
        PARSE_ARGV 0 MODULE
        "${options}" "${one_value_args}" "${multi_value_args}")

    if(NOT MODULE_MODULE_NAME)
        message(FATAL_ERROR "[BuildModule] MODULE_NAME is required")
    endif()
    if(TARGET "${MODULE_MODULE_NAME}")
        message(FATAL_ERROR
            "[BuildModule] Target already exists: ${MODULE_MODULE_NAME}")
    endif()
    if(MODULE_STATIC_LIB AND MODULE_SHARED_LIB)
        message(FATAL_ERROR
            "[BuildModule:${MODULE_MODULE_NAME}] STATIC_LIB and SHARED_LIB "
            "are mutually exclusive")
    endif()
    if(NOT MODULE_TARGET_SOURCE_PRIVATE)
        message(FATAL_ERROR
            "[BuildModule:${MODULE_MODULE_NAME}] TARGET_SOURCE_PRIVATE is required; "
            "a compiled library needs at least one source file")
    endif()
    if(MODULE_TARGET_SOURCE_PUBLIC_HEADER
       AND NOT MODULE_TARGET_SOURCE_HEADER_BASE_DIR)
        message(FATAL_ERROR
            "[BuildModule:${MODULE_MODULE_NAME}] "
            "TARGET_SOURCE_HEADER_BASE_DIR is required when "
            "TARGET_SOURCE_PUBLIC_HEADER is specified")
    endif()
    if(MODULE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[BuildModule:${MODULE_MODULE_NAME}] Unknown arguments: "
            "${MODULE_UNPARSED_ARGUMENTS}")
    endif()

    if(MODULE_STATIC_LIB)
        set(_library_type STATIC)
    else()
        # Preserve the historical default while allowing SHARED_LIB to make the
        # caller's intent explicit.
        set(_library_type SHARED)
    endif()

    message(STATUS
        "[BuildModule] ${MODULE_MODULE_NAME} (${_library_type})")
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS
            "[BuildModule]   Sources      : ${MODULE_TARGET_SOURCE_PRIVATE}")
        message(STATUS
            "[BuildModule]   Headers      : ${MODULE_TARGET_SOURCE_PUBLIC_HEADER}")
        message(STATUS
            "[BuildModule]   Header Base  : ${MODULE_TARGET_SOURCE_HEADER_BASE_DIR}")
        message(STATUS
            "[BuildModule]   Link Private : ${MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE}")
        message(STATUS
            "[BuildModule]   Link Public  : ${MODULE_TARGET_ADD_LINK_LIBRARY_PUBLIC}")
        message(STATUS
            "[BuildModule]   Dependencies : ${MODULE_TARGET_ADD_DEPENDENCIES}")
        message(STATUS
            "[BuildModule]   Def Private  : ${MODULE_TARGET_PRIVATE_DEFINITIONS}")
        message(STATUS
            "[BuildModule]   Def Public   : ${MODULE_TARGET_PUBLIC_DEFINITIONS}")
        message(STATUS
            "[BuildModule]   IDE Folder   : ${MODULE_IDE_FOLDER}")
        message(STATUS
            "[BuildModule]   Include Build: ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}")
        message(STATUS
            "[BuildModule]   Include Inst : ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}")
        message(STATUS
            "[BuildModule]   Include Priv : ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}")
    endif()

    add_library("${MODULE_MODULE_NAME}" ${_library_type} "")
    target_sources("${MODULE_MODULE_NAME}"
        PRIVATE ${MODULE_TARGET_SOURCE_PRIVATE}
        PUBLIC FILE_SET HEADERS
        BASE_DIRS ${MODULE_TARGET_SOURCE_HEADER_BASE_DIR}
        FILES ${MODULE_TARGET_SOURCE_PUBLIC_HEADER}
    )

    set(_defaults_arguments TARGET "${MODULE_MODULE_NAME}")
    if(MODULE_IDE_FOLDER)
        list(APPEND _defaults_arguments IDE_FOLDER "${MODULE_IDE_FOLDER}")
    endif()
    _tt_apply_cpp_library_defaults(${_defaults_arguments})
    _tt_apply_target_usage_requirements(
        TARGET "${MODULE_MODULE_NAME}"
        PRIVATE_LINK_LIBRARIES ${MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE}
        PUBLIC_LINK_LIBRARIES ${MODULE_TARGET_ADD_LINK_LIBRARY_PUBLIC}
        DEPENDENCIES ${MODULE_TARGET_ADD_DEPENDENCIES}
        BUILD_INTERFACE_DIRECTORIES
            ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}
        INSTALL_INTERFACE_DIRECTORIES
            ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}
        PRIVATE_INCLUDE_DIRECTORIES
            ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}
        PUBLIC_COMPILE_DEFINITIONS
            ${MODULE_TARGET_PUBLIC_DEFINITIONS}
        PRIVATE_COMPILE_DEFINITIONS
            ${MODULE_TARGET_PRIVATE_DEFINITIONS}
    )

    set(_finalize_options)
    if(MODULE_NO_INSTALL)
        list(APPEND _finalize_options NO_INSTALL)
    endif()
    _tt_finalize_library_target(
        ${_finalize_options}
        TARGET "${MODULE_MODULE_NAME}"
        LIBRARY_TYPE "${_library_type}"
        FILE_DESCRIPTION "${MODULE_MODULE_NAME} Library"
    )
endfunction()
