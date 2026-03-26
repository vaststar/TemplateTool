include_guard()

include(GenerateAppInfoFiles)

function(BuildLinuxCPackModule)
    message(STATUS "====Start Build Linux CPack Config Module====")

    set(options)
    set(oneValueArg OUTPUT_CONFIG_VAR)
    set(multiValueArgs)
    cmake_parse_arguments(MODULE "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    if(NOT GLOBAL_CPACK_LINUX_TEMPLATE)
        message(WARNING "GLOBAL_CPACK_LINUX_TEMPLATE not set, skipping Linux CPack config generation")
        return()
    endif()

    set(CPACK_CONFIG_PATH ${CMAKE_CURRENT_BINARY_DIR}/cpack_linux_config.cmake)

    message(STATUS "  Template : ${GLOBAL_CPACK_LINUX_TEMPLATE}")
    message(STATUS "  Output   : ${CPACK_CONFIG_PATH}")

    message(STATUS "***generate cpack_linux_config.cmake***")

    generate_app_info_files(
        INPUT_JSON_FILE ${GLOBAL_APP_VERSION_JSON}
        INPUT_JSON_TARGET ${GLOBAL_APP_VERSION_JSON_TARGET}
        INPUT_VERSION_TEMPLATE ${GLOBAL_CPACK_LINUX_TEMPLATE}
        OUTPUT_FILE ${CPACK_CONFIG_PATH}
        INTERNAL_NAME ""
        FILE_DESCRIPTION ""
        ORIGINAL_FILENAME ""
        OUTPUT_TARGET_VAR CPACK_LINUX_CONFIG_TARGET
    )

    # Export the generated config path to caller
    if(MODULE_OUTPUT_CONFIG_VAR)
        set(${MODULE_OUTPUT_CONFIG_VAR} ${CPACK_CONFIG_PATH} PARENT_SCOPE)
    endif()
endfunction()
