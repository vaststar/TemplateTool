include_guard()

include(GenerateAppInfoFiles)

function(BuildRCFileModule)
    set(one_value_args MODULE_NAME FILE_DESCRIPTION)
    cmake_parse_arguments(PARSE_ARGV 0 MODULE "" "${one_value_args}" "")

    if(NOT MODULE_MODULE_NAME)
        message(FATAL_ERROR "[BuildRCFileModule] MODULE_NAME is required")
    endif()
    if(NOT TARGET "${MODULE_MODULE_NAME}")
        message(FATAL_ERROR
            "[BuildRCFileModule] Target does not exist: ${MODULE_MODULE_NAME}")
    endif()
    if(NOT MODULE_FILE_DESCRIPTION)
        message(FATAL_ERROR
            "[BuildRCFileModule] FILE_DESCRIPTION is required for "
            "${MODULE_MODULE_NAME}")
    endif()
    if(MODULE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[BuildRCFileModule] Unknown arguments: ${MODULE_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT GLOBAL_APP_VERSION_JSON)
        message(FATAL_ERROR
            "[BuildRCFileModule] GLOBAL_APP_VERSION_JSON is not configured")
    endif()
    if(NOT GLOBAL_APP_VERSION_JSON_TARGET
       OR NOT TARGET "${GLOBAL_APP_VERSION_JSON_TARGET}")
        message(FATAL_ERROR
            "[BuildRCFileModule] GLOBAL_APP_VERSION_JSON_TARGET is missing or "
            "does not name an existing target")
    endif()
    if(NOT GLOBAL_APP_RC_TEMPLATE OR NOT EXISTS "${GLOBAL_APP_RC_TEMPLATE}")
        message(FATAL_ERROR
            "[BuildRCFileModule] GLOBAL_APP_RC_TEMPLATE is missing or does not "
            "name an existing file: ${GLOBAL_APP_RC_TEMPLATE}")
    endif()

    set(app_rc_path
        "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_MODULE_NAME}_resource.rc")

    message(STATUS
        "[BuildRCFileModule] ${MODULE_MODULE_NAME} -> ${app_rc_path}")
    if(CMAKE_VERBOSE_MAKEFILE)
        message(STATUS
            "[BuildRCFileModule]   Description: ${MODULE_FILE_DESCRIPTION}")
        message(STATUS
            "[BuildRCFileModule]   Template   : ${GLOBAL_APP_RC_TEMPLATE}")
    endif()

    generate_app_info_files(
        INPUT_JSON_FILE "${GLOBAL_APP_VERSION_JSON}"
        INPUT_JSON_TARGET "${GLOBAL_APP_VERSION_JSON_TARGET}"
        INPUT_VERSION_TEMPLATE "${GLOBAL_APP_RC_TEMPLATE}"
        OUTPUT_FILE "${app_rc_path}"
        INTERNAL_NAME "${MODULE_MODULE_NAME}"
        FILE_DESCRIPTION "${MODULE_FILE_DESCRIPTION}"
        ORIGINAL_FILENAME "$<TARGET_FILE_NAME:${MODULE_MODULE_NAME}>"
        OUTPUT_TARGET_VAR app_rc_target
    )

    target_sources("${MODULE_MODULE_NAME}" PRIVATE "${app_rc_path}")
    add_dependencies("${MODULE_MODULE_NAME}" "${app_rc_target}")
endfunction()
