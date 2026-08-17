include_guard()

include(GenerateAppInfoFiles)
include(GetRequiredTargetProperty)

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
    get_required_target_property(
        TARGET AppVersionMetadata
        PROPERTY UCF_APP_VERSION_JSON_FILE
        OUTPUT_VARIABLE app_version_json
    )
    get_required_target_property(
        TARGET AppVersionMetadata
        PROPERTY UCF_APP_VERSION_GENERATOR_TARGET
        OUTPUT_VARIABLE app_version_target
    )
    get_required_target_property(
        TARGET AppVersionMetadata
        PROPERTY UCF_APP_LINUX_DESKTOP_TEMPLATE
        OUTPUT_VARIABLE app_desktop_template
    )
    if(NOT TARGET "${app_version_target}")
        message(FATAL_ERROR
            "[BuildLinuxDesktopModule] AppVersionMetadata generator target does "
            "not exist: ${app_version_target}")
    endif()
    if(NOT EXISTS "${app_desktop_template}")
        message(FATAL_ERROR
            "[BuildLinuxDesktopModule] desktop template does not exist: "
            "${app_desktop_template}")
    endif()

    set(desktop_file_path
        "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_MODULE_NAME}.desktop")

    message(STATUS
        "[BuildLinuxDesktopModule] ${MODULE_MODULE_NAME} -> ${desktop_file_path}")
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS
            "[BuildLinuxDesktopModule]   Description: ${MODULE_FILE_DESCRIPTION}")
        message(STATUS
            "[BuildLinuxDesktopModule]   Template   : "
            "${app_desktop_template}")
        message(STATUS
            "[BuildLinuxDesktopModule]   Icon       : ${MODULE_ICON_FILE}")
    endif()

    generate_app_info_files(
        INPUT_JSON_FILE "${app_version_json}"
        INPUT_JSON_TARGET "${app_version_target}"
        INPUT_VERSION_TEMPLATE "${app_desktop_template}"
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
