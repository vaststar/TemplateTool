include_guard()

include(GenerateAppInfoFiles)

function(BuildBundlePListModule)
        message(STATUS "====Start Build Plist File Module====")
        set(options)
        set(oneValueArg MODULE_NAME FILE_DESCRIPTION)
        set(multiValueArgs )
        cmake_parse_arguments(MODULE "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

        message(STATUS "Parse Args Results:")
        message(STATUS "MODULE_NAME: ${MODULE_MODULE_NAME}")
        message(STATUS "FILE_DESCRIPTION: ${MODULE_FILE_DESCRIPTION}")

        message(STATUS "***create info.plist file for: ${MODULE_MODULE_NAME}***")

        set(APP_RC_PATH ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)

        if(NOT EXISTS "${APP_RC_PATH}")
            file(WRITE "${APP_RC_PATH}" "<dummy plist>")
        endif()

        set_target_properties(${MODULE_MODULE_NAME} PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST ${APP_RC_PATH}
        )

        generate_app_info_files(
            INPUT_JSON_FILE ${GLOBAL_APP_VERSION_JSON}
            INPUT_JSON_TARGET ${GLOBAL_APP_VERSION_JSON_TARGET}
            INPUT_VERSION_TEMPLATE ${GLOBAL_APP_RC_TEMPLATE}
            OUTPUT_FILE ${APP_RC_PATH}
            INTERNAL_NAME ${MODULE_MODULE_NAME}
            FILE_DESCRIPTION ${MODULE_FILE_DESCRIPTION}
            ORIGINAL_FILENAME $<TARGET_FILE_NAME:${MODULE_MODULE_NAME}>
            OUTPUT_TARGET_VAR APP_RC_TARGET
        )
        add_dependencies(${MODULE_MODULE_NAME} ${APP_RC_TARGET})

        add_custom_command(TARGET ${MODULE_MODULE_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${APP_RC_PATH}"
                "$<TARGET_BUNDLE_DIR:${MODULE_MODULE_NAME}>/Contents/Info.plist"
            COMMENT "Copying updated Info.plist into app bundle"
        )
endfunction()