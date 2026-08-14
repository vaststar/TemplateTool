include_guard()

include("${CMAKE_CURRENT_LIST_DIR}/../BuildInstallModule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../LinkTargetIncludeDirectories.cmake")

if(WIN32)
    include("${CMAKE_CURRENT_LIST_DIR}/../BuildRCFileModule.cmake")
endif()

function(_tt_apply_cpp_library_defaults)
    set(one_value_args TARGET IDE_FOLDER)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "${one_value_args}" "")

    if(NOT ARG_TARGET)
        message(FATAL_ERROR
            "[_tt_apply_cpp_library_defaults] TARGET is required")
    endif()
    if(NOT TARGET "${ARG_TARGET}")
        message(FATAL_ERROR
            "[_tt_apply_cpp_library_defaults] Unknown target: ${ARG_TARGET}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[_tt_apply_cpp_library_defaults:${ARG_TARGET}] Unknown arguments: "
            "${ARG_UNPARSED_ARGUMENTS}")
    endif()

    target_compile_features("${ARG_TARGET}" PUBLIC cxx_std_20)
    target_compile_definitions("${ARG_TARGET}" PRIVATE
        CMAKE_VERSION_STR="${CMAKE_VERSION}"
        CMAKE_COMPILER_ID_STR="${CMAKE_CXX_COMPILER_ID}"
        CMAKE_COMPILER_VERSION_STR="${CMAKE_CXX_COMPILER_VERSION}"
        CMAKE_COMPILER_PATH_STR="${CMAKE_CXX_COMPILER}"
    )

    set_target_properties("${ARG_TARGET}" PROPERTIES
        CXX_EXTENSIONS OFF
    )

    if(ARG_IDE_FOLDER)
        set_target_properties("${ARG_TARGET}" PROPERTIES
            FOLDER "${ARG_IDE_FOLDER}"
        )
    endif()
endfunction()

function(_tt_apply_shared_library_runtime_defaults)
    set(one_value_args TARGET)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "${one_value_args}" "")

    if(NOT ARG_TARGET)
        message(FATAL_ERROR
            "[_tt_apply_shared_library_runtime_defaults] TARGET is required")
    endif()
    if(NOT TARGET "${ARG_TARGET}")
        message(FATAL_ERROR
            "[_tt_apply_shared_library_runtime_defaults] Unknown target: "
            "${ARG_TARGET}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[_tt_apply_shared_library_runtime_defaults:${ARG_TARGET}] "
            "Unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    get_target_property(actual_target_type "${ARG_TARGET}" TYPE)
    if(NOT "${actual_target_type}" STREQUAL "SHARED_LIBRARY")
        message(FATAL_ERROR
            "[_tt_apply_shared_library_runtime_defaults:${ARG_TARGET}] "
            "Expected SHARED_LIBRARY, got '${actual_target_type}'")
    endif()

    if(APPLE)
        set_target_properties("${ARG_TARGET}" PROPERTIES
            INSTALL_NAME_DIR "@rpath"
            INSTALL_RPATH "@loader_path;@executable_path"
            BUILD_WITH_INSTALL_RPATH OFF
            INSTALL_RPATH_USE_LINK_PATH OFF
        )
    elseif(UNIX)
        set_target_properties("${ARG_TARGET}" PROPERTIES
            INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib"
            BUILD_WITH_INSTALL_RPATH OFF
            INSTALL_RPATH_USE_LINK_PATH OFF
        )
    endif()
endfunction()

function(_tt_apply_target_usage_requirements)
    set(one_value_args TARGET)
    set(multi_value_args
        PRIVATE_LINK_LIBRARIES
        PUBLIC_LINK_LIBRARIES
        DEPENDENCIES
        BUILD_INTERFACE_DIRECTORIES
        INSTALL_INTERFACE_DIRECTORIES
        PRIVATE_INCLUDE_DIRECTORIES
        PUBLIC_COMPILE_DEFINITIONS
        PRIVATE_COMPILE_DEFINITIONS
    )
    cmake_parse_arguments(
        PARSE_ARGV 0 ARG "" "${one_value_args}" "${multi_value_args}")

    if(NOT ARG_TARGET)
        message(FATAL_ERROR
            "[_tt_apply_target_usage_requirements] TARGET is required")
    endif()
    if(NOT TARGET "${ARG_TARGET}")
        message(FATAL_ERROR
            "[_tt_apply_target_usage_requirements] Unknown target: ${ARG_TARGET}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[_tt_apply_target_usage_requirements:${ARG_TARGET}] Unknown arguments: "
            "${ARG_UNPARSED_ARGUMENTS}")
    endif()

    LinkTargetIncludeDirectories(
        MODULE_NAME "${ARG_TARGET}"
        TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
            ${ARG_BUILD_INTERFACE_DIRECTORIES}
        TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
            ${ARG_INSTALL_INTERFACE_DIRECTORIES}
        TARGET_INCLUDE_DIRECTORIES_PRIVATE
            ${ARG_PRIVATE_INCLUDE_DIRECTORIES}
    )

    if(ARG_PRIVATE_LINK_LIBRARIES)
        target_link_libraries("${ARG_TARGET}" PRIVATE
            ${ARG_PRIVATE_LINK_LIBRARIES})
    endif()
    if(ARG_PUBLIC_LINK_LIBRARIES)
        target_link_libraries("${ARG_TARGET}" PUBLIC
            ${ARG_PUBLIC_LINK_LIBRARIES})
    endif()
    if(ARG_DEPENDENCIES)
        add_dependencies("${ARG_TARGET}" ${ARG_DEPENDENCIES})
    endif()
    if(ARG_PUBLIC_COMPILE_DEFINITIONS)
        target_compile_definitions("${ARG_TARGET}" PUBLIC
            ${ARG_PUBLIC_COMPILE_DEFINITIONS})
    endif()
    if(ARG_PRIVATE_COMPILE_DEFINITIONS)
        target_compile_definitions("${ARG_TARGET}" PRIVATE
            ${ARG_PRIVATE_COMPILE_DEFINITIONS})
    endif()
endfunction()

function(_tt_finalize_library_target)
    set(options NO_INSTALL)
    set(one_value_args TARGET LIBRARY_TYPE FILE_DESCRIPTION)
    cmake_parse_arguments(
        PARSE_ARGV 0 ARG "${options}" "${one_value_args}" "")

    if(NOT ARG_TARGET)
        message(FATAL_ERROR
            "[_tt_finalize_library_target] TARGET is required")
    endif()
    if(NOT TARGET "${ARG_TARGET}")
        message(FATAL_ERROR
            "[_tt_finalize_library_target] Unknown target: ${ARG_TARGET}")
    endif()
    if(NOT "${ARG_LIBRARY_TYPE}" MATCHES "^(STATIC|SHARED)$")
        message(FATAL_ERROR
            "[_tt_finalize_library_target:${ARG_TARGET}] LIBRARY_TYPE must be "
            "STATIC or SHARED, got '${ARG_LIBRARY_TYPE}'")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[_tt_finalize_library_target:${ARG_TARGET}] Unknown arguments: "
            "${ARG_UNPARSED_ARGUMENTS}")
    endif()

    get_target_property(actual_target_type "${ARG_TARGET}" TYPE)
    if(ARG_LIBRARY_TYPE STREQUAL "STATIC")
        set(expected_target_type STATIC_LIBRARY)
    else()
        set(expected_target_type SHARED_LIBRARY)
    endif()
    if(NOT "${actual_target_type}" STREQUAL "${expected_target_type}")
        message(FATAL_ERROR
            "[_tt_finalize_library_target:${ARG_TARGET}] Declared library type "
            "${ARG_LIBRARY_TYPE} does not match target type "
            "${actual_target_type}")
    endif()

    if(ARG_LIBRARY_TYPE STREQUAL "SHARED")
        _tt_apply_shared_library_runtime_defaults(TARGET "${ARG_TARGET}")
    endif()

    if(NOT ARG_NO_INSTALL)
        BuildInstallModule(MODULE_NAME "${ARG_TARGET}")
    endif()

    if(WIN32 AND ARG_LIBRARY_TYPE STREQUAL "SHARED")
        if(NOT ARG_FILE_DESCRIPTION)
            message(FATAL_ERROR
                "[_tt_finalize_library_target:${ARG_TARGET}] "
                "FILE_DESCRIPTION is required for a Windows shared library")
        endif()
        BuildRCFileModule(
            MODULE_NAME "${ARG_TARGET}"
            FILE_DESCRIPTION "${ARG_FILE_DESCRIPTION}"
        )
    endif()
endfunction()
