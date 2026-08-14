include_guard()

include(GenerateAppInfoFiles)

function(BuildLinuxDesktopModule)
    set(one_value_args MODULE_NAME FILE_DESCRIPTION ICON_FILE)
    cmake_parse_arguments(PARSE_ARGV 0 MODULE "" "${one_value_args}" "")

    if(NOT MODULE_MODULE_NAME)
        message(FATAL_ERROR "[BuildLinuxDesktopModule] MODULE_NAME is required")
    endif()
    if(NOT TARGET "${MODULE_MODULE_NAME}")
        message(FATAL_ERROR
            "[BuildLinuxDesktopModule] Target does not exist: "
            "${MODULE_MODULE_NAME}")
    endif()
    if(NOT MODULE_FILE_DESCRIPTION)
        message(FATAL_ERROR
            "[BuildLinuxDesktopModule] FILE_DESCRIPTION is required for "
            "${MODULE_MODULE_NAME}")
    endif()
    if(MODULE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[BuildLinuxDesktopModule] Unknown arguments: "
            "${MODULE_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT GLOBAL_APP_VERSION_JSON)
        message(FATAL_ERROR
            "[BuildLinuxDesktopModule] GLOBAL_APP_VERSION_JSON is not configured")
    endif()
    if(NOT GLOBAL_APP_VERSION_JSON_TARGET
       OR NOT TARGET "${GLOBAL_APP_VERSION_JSON_TARGET}")
        message(FATAL_ERROR
            "[BuildLinuxDesktopModule] GLOBAL_APP_VERSION_JSON_TARGET is missing "
            "or does not name an existing target")
    endif()
    if(NOT GLOBAL_APP_DESKTOP_TEMPLATE
       OR NOT EXISTS "${GLOBAL_APP_DESKTOP_TEMPLATE}")
        message(FATAL_ERROR
            "[BuildLinuxDesktopModule] GLOBAL_APP_DESKTOP_TEMPLATE is missing "
            "or does not name an existing file: ${GLOBAL_APP_DESKTOP_TEMPLATE}")
    endif()

    set(desktop_file_path
        "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_MODULE_NAME}.desktop")

    message(STATUS
        "[BuildLinuxDesktopModule] ${MODULE_MODULE_NAME} -> ${desktop_file_path}")
    if(CMAKE_VERBOSE_MAKEFILE)
        message(STATUS
            "[BuildLinuxDesktopModule]   Description: ${MODULE_FILE_DESCRIPTION}")
        message(STATUS
            "[BuildLinuxDesktopModule]   Template   : "
            "${GLOBAL_APP_DESKTOP_TEMPLATE}")
        message(STATUS
            "[BuildLinuxDesktopModule]   Icon       : ${MODULE_ICON_FILE}")
    endif()

    generate_app_info_files(
        INPUT_JSON_FILE "${GLOBAL_APP_VERSION_JSON}"
        INPUT_JSON_TARGET "${GLOBAL_APP_VERSION_JSON_TARGET}"
        INPUT_VERSION_TEMPLATE "${GLOBAL_APP_DESKTOP_TEMPLATE}"
        OUTPUT_FILE "${desktop_file_path}"
        INTERNAL_NAME "${MODULE_MODULE_NAME}"
        FILE_DESCRIPTION "${MODULE_FILE_DESCRIPTION}"
        ORIGINAL_FILENAME "${MODULE_MODULE_NAME}"
        OUTPUT_TARGET_VAR desktop_file_target
    )
    add_dependencies("${MODULE_MODULE_NAME}" "${desktop_file_target}")

    # Install relative to CMAKE_INSTALL_PREFIX. DEB packaging later relocates
    # these files to /usr/share so desktop environments can discover them.
    install(FILES "${desktop_file_path}"
        DESTINATION share/applications
    )

    if(MODULE_ICON_FILE)
        if(EXISTS "${MODULE_ICON_FILE}")
            install(FILES "${MODULE_ICON_FILE}"
                DESTINATION share/icons/hicolor/256x256/apps
                RENAME "${MODULE_MODULE_NAME}.png"
            )
        else()
            message(WARNING
                "[BuildLinuxDesktopModule] Icon not found: ${MODULE_ICON_FILE}")
        endif()
    endif()
endfunction()
