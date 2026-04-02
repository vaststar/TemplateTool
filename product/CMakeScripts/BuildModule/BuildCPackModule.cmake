include_guard()

include(GenerateAppInfoFiles)

# ==========================================
# BuildCPackModule - Unified CPack config generator (all platforms)
# ==========================================
# Generates platform-specific CPack config from templates using the same
# GenerateAppInfoFiles pipeline as RC/PList/.desktop generation.
#
# Usage:
#   BuildCPackModule(
#       TEMPLATE  /path/to/cpack_xxx_config.cmake.in
#       OUTPUT_CONFIG_VAR  MY_VAR
#   )
# ==========================================
function(BuildCPackModule)
    message(STATUS "====Start Build CPack Config Module====")

    set(options)
    set(oneValueArg TEMPLATE OUTPUT_CONFIG_VAR)
    set(multiValueArgs)
    cmake_parse_arguments(MODULE "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    if(NOT MODULE_TEMPLATE)
        message(WARNING "TEMPLATE not provided, skipping CPack config generation")
        return()
    endif()

    if(NOT EXISTS "${MODULE_TEMPLATE}")
        message(WARNING "CPack template not found: ${MODULE_TEMPLATE}")
        return()
    endif()

    get_filename_component(TEMPLATE_NAME "${MODULE_TEMPLATE}" NAME_WE)
    set(CPACK_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR}/${TEMPLATE_NAME}.cmake)

    message(STATUS "  Template : ${MODULE_TEMPLATE}")
    message(STATUS "  Output   : ${CPACK_CONFIG_PATH}")

    message(STATUS "***generate ${TEMPLATE_NAME}.cmake***")

    generate_app_info_files(
        INPUT_JSON_FILE ${GLOBAL_APP_VERSION_JSON}
        INPUT_JSON_TARGET ${GLOBAL_APP_VERSION_JSON_TARGET}
        INPUT_VERSION_TEMPLATE ${MODULE_TEMPLATE}
        OUTPUT_FILE ${CPACK_CONFIG_PATH}
        INTERNAL_NAME ""
        FILE_DESCRIPTION ""
        ORIGINAL_FILENAME ""
        OUTPUT_TARGET_VAR CPACK_CONFIG_TARGET
    )

    # Export the generated config path to caller
    if(MODULE_OUTPUT_CONFIG_VAR)
        set(${MODULE_OUTPUT_CONFIG_VAR} ${CPACK_CONFIG_PATH} PARENT_SCOPE)
    endif()
endfunction()
