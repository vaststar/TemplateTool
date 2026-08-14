include_guard()

include(GenerateAppInfoFiles)

function(BuildBundlePListModule)
    set(one_value_args MODULE_NAME FILE_DESCRIPTION)
    cmake_parse_arguments(PARSE_ARGV 0 MODULE "" "${one_value_args}" "")

    if(NOT MODULE_MODULE_NAME)
        message(FATAL_ERROR "[BuildBundlePListModule] MODULE_NAME is required")
    endif()
    if(NOT TARGET "${MODULE_MODULE_NAME}")
        message(FATAL_ERROR
            "[BuildBundlePListModule] Target does not exist: "
            "${MODULE_MODULE_NAME}")
    endif()
    if(NOT MODULE_FILE_DESCRIPTION)
        message(FATAL_ERROR
            "[BuildBundlePListModule] FILE_DESCRIPTION is required for "
            "${MODULE_MODULE_NAME}")
    endif()
    if(MODULE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[BuildBundlePListModule] Unknown arguments: "
            "${MODULE_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT GLOBAL_APP_VERSION_JSON)
        message(FATAL_ERROR
            "[BuildBundlePListModule] GLOBAL_APP_VERSION_JSON is not configured")
    endif()
    if(NOT GLOBAL_APP_VERSION_JSON_TARGET
       OR NOT TARGET "${GLOBAL_APP_VERSION_JSON_TARGET}")
        message(FATAL_ERROR
            "[BuildBundlePListModule] GLOBAL_APP_VERSION_JSON_TARGET is missing "
            "or does not name an existing target")
    endif()
    if(NOT GLOBAL_APP_RC_TEMPLATE OR NOT EXISTS "${GLOBAL_APP_RC_TEMPLATE}")
        message(FATAL_ERROR
            "[BuildBundlePListModule] GLOBAL_APP_RC_TEMPLATE is missing or does "
            "not name an existing file: ${GLOBAL_APP_RC_TEMPLATE}")
    endif()

    set(plist_path
        "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_MODULE_NAME}_Info.plist")
    set(plist_placeholder_path
        "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_MODULE_NAME}_Info.plist.in")

    message(STATUS
        "[BuildBundlePListModule] ${MODULE_MODULE_NAME} -> ${plist_path}")
    if(CMAKE_VERBOSE_MAKEFILE)
        message(STATUS
            "[BuildBundlePListModule]   Description: ${MODULE_FILE_DESCRIPTION}")
        message(STATUS
            "[BuildBundlePListModule]   Template   : ${GLOBAL_APP_RC_TEMPLATE}")
    endif()

    # CMake requires its plist template to exist during generation. Keep that
    # placeholder separate from the build-time output so it cannot make the
    # real generation command appear up to date on a clean build.
    if(NOT EXISTS "${plist_placeholder_path}")
        file(WRITE "${plist_placeholder_path}"
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<plist version=\"1.0\"><dict/></plist>\n")
    endif()

    set_target_properties("${MODULE_MODULE_NAME}" PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${plist_placeholder_path}"
    )

    generate_app_info_files(
        INPUT_JSON_FILE "${GLOBAL_APP_VERSION_JSON}"
        INPUT_JSON_TARGET "${GLOBAL_APP_VERSION_JSON_TARGET}"
        INPUT_VERSION_TEMPLATE "${GLOBAL_APP_RC_TEMPLATE}"
        OUTPUT_FILE "${plist_path}"
        INTERNAL_NAME "${MODULE_MODULE_NAME}"
        FILE_DESCRIPTION "${MODULE_FILE_DESCRIPTION}"
        ORIGINAL_FILENAME "$<TARGET_FILE_NAME:${MODULE_MODULE_NAME}>"
        OUTPUT_TARGET_VAR plist_target
    )
    add_dependencies("${MODULE_MODULE_NAME}" "${plist_target}")

    add_custom_command(TARGET "${MODULE_MODULE_NAME}" POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${plist_path}"
            "$<TARGET_BUNDLE_DIR:${MODULE_MODULE_NAME}>/Contents/Info.plist"
        COMMENT "Copying updated Info.plist into app bundle"
    )
endfunction()
