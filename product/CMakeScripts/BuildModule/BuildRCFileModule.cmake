include_guard()

include(GenerateAppRCFiles)

function(BuildRCFileModule)
        message(STATUS "====Start Build RC File Module====")
        set(options)
        set(oneValueArg MODULE_NAME FILE_DESCRIPTION)
        set(multiValueArgs )
        cmake_parse_arguments(MODULE "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

        message(STATUS "Parse Args Results:")
        message(STATUS "MODULE_NAME: ${MODULE_MODULE_NAME}")
        message(STATUS "FILE_DESCRIPTION: ${MODULE_FILE_DESCRIPTION}")

        message(STATUS "***create rc file for: ${MODULE_MODULE_NAME}***")

        set(APP_RC_PATH ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_MODULE_NAME}_resource.rc)
        generate_app_rc_files(
            INPUT_JSON_FILE ${GLOBAL_APP_VERSION_JSON}
            INPUT_JSON_TARGET ${GLOBAL_APP_VERSION_JSON_TARGET}
            INPUT_VERSION_TEMPLATE ${GLOBAL_APP_RC_TEMPLATE}
            OUTPUT_FILE ${APP_RC_PATH}
            INTERNAL_NAME ${MODULE_MODULE_NAME}
            FILE_DESCRIPTION ${MODULE_FILE_DESCRIPTION}
            ORIGINAL_FILENAME $<TARGET_FILE_NAME:${MODULE_MODULE_NAME}>
            APP_RC_TARGET
        )
        target_sources(${MODULE_MODULE_NAME} PRIVATE ${APP_RC_PATH})
        add_dependencies(${MODULE_MODULE_NAME} ${APP_RC_TARGET})
endfunction()