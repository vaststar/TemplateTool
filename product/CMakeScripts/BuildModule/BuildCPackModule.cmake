include_guard()

include(GenerateAppInfoFiles)

# ==========================================
# BuildCPackModule - Unified CPack config generator (all platforms)
# ==========================================
# Generates platform-specific CPack config from templates using the same
# GenerateAppInfoFiles pipeline as RC/PList/.desktop generation.
#
# The generated config is read by cpack at packaging time via
# CPACK_PROJECT_CONFIG_FILE, not at configure time. This avoids the
# chicken-and-egg problem where version_meta.json is only available
# after the first build.
#
# Usage:
#   BuildCPackModule(
#       TEMPLATE         /path/to/cpack_xxx_config.cmake.in
#       PRE_BUILD_SCRIPT /path/to/cpack_pre_build.cmake  (optional)
#       OUTPUT_CONFIG_VAR  MY_VAR
#   )
# ==========================================
function(BuildCPackModule)
    set(one_value_args TEMPLATE OUTPUT_CONFIG_VAR PRE_BUILD_SCRIPT)
    cmake_parse_arguments(
        PARSE_ARGV 0 MODULE "" "${one_value_args}" "")

    if(NOT MODULE_TEMPLATE)
        message(FATAL_ERROR "[BuildCPackModule] TEMPLATE is required")
    endif()
    if(NOT EXISTS "${MODULE_TEMPLATE}")
        message(FATAL_ERROR
            "[BuildCPackModule] Template does not exist: ${MODULE_TEMPLATE}")
    endif()
    if(MODULE_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[BuildCPackModule] Arguments missing values: "
            "${MODULE_KEYWORDS_MISSING_VALUES}")
    endif()
    if(MODULE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[BuildCPackModule] Unknown arguments: "
            "${MODULE_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT GLOBAL_APP_VERSION_JSON)
        message(FATAL_ERROR
            "[BuildCPackModule] GLOBAL_APP_VERSION_JSON is not configured")
    endif()
    if(NOT GLOBAL_APP_VERSION_JSON_TARGET
       OR NOT TARGET "${GLOBAL_APP_VERSION_JSON_TARGET}")
        message(FATAL_ERROR
            "[BuildCPackModule] GLOBAL_APP_VERSION_JSON_TARGET is missing or "
            "does not name an existing target")
    endif()
    if(MODULE_PRE_BUILD_SCRIPT
       AND NOT EXISTS "${MODULE_PRE_BUILD_SCRIPT}")
        message(FATAL_ERROR
            "[BuildCPackModule] PRE_BUILD_SCRIPT does not exist: "
            "${MODULE_PRE_BUILD_SCRIPT}")
    endif()

    get_filename_component(TEMPLATE_NAME "${MODULE_TEMPLATE}" NAME_WE)
    set(CPACK_CONFIG_PATH
        "${CMAKE_CURRENT_BINARY_DIR}/${TEMPLATE_NAME}.cmake")

    message(STATUS
        "[BuildCPackModule] ${MODULE_TEMPLATE} -> ${CPACK_CONFIG_PATH}")

    # Build-time generation via add_custom_command
    generate_app_info_files(
        INPUT_JSON_FILE "${GLOBAL_APP_VERSION_JSON}"
        INPUT_JSON_TARGET "${GLOBAL_APP_VERSION_JSON_TARGET}"
        INPUT_VERSION_TEMPLATE "${MODULE_TEMPLATE}"
        OUTPUT_FILE "${CPACK_CONFIG_PATH}"
        INTERNAL_NAME ""
        FILE_DESCRIPTION ""
        ORIGINAL_FILENAME ""
        OUTPUT_TARGET_VAR cpack_config_target
    )

    # Tell CPack to read this file at cpack runtime (not configure time)
    set(CPACK_PROJECT_CONFIG_FILE "${CPACK_CONFIG_PATH}" PARENT_SCOPE)
    if(CMAKE_VERBOSE_MAKEFILE)
        message(STATUS
            "[BuildCPackModule]   Generated target: ${cpack_config_target}")
        message(STATUS
            "[BuildCPackModule]   Project config  : ${CPACK_CONFIG_PATH}")
    endif()

    # Set pre-build script to strip dev files before packaging
    if(MODULE_PRE_BUILD_SCRIPT)
        set(CPACK_PRE_BUILD_SCRIPTS "${MODULE_PRE_BUILD_SCRIPT}" PARENT_SCOPE)
        if(CMAKE_VERBOSE_MAKEFILE)
            message(STATUS
                "[BuildCPackModule]   Pre-build script: "
                "${MODULE_PRE_BUILD_SCRIPT}")
        endif()
    endif()

    # Export path for reference
    if(MODULE_OUTPUT_CONFIG_VAR)
        if(NOT "${MODULE_OUTPUT_CONFIG_VAR}"
           MATCHES "^[A-Za-z_][A-Za-z0-9_]*$")
            message(FATAL_ERROR
                "[BuildCPackModule] OUTPUT_CONFIG_VAR must be a valid "
                "variable name, got '${MODULE_OUTPUT_CONFIG_VAR}'")
        endif()
        set("${MODULE_OUTPUT_CONFIG_VAR}"
            "${CPACK_CONFIG_PATH}"
            PARENT_SCOPE
        )
    endif()
endfunction()
