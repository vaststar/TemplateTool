include_guard()

include(GenerateAppInfoFiles)

function(BuildLinuxDesktopModule)
    message(STATUS "====Start Build Linux Desktop File Module====")
    set(options)
    set(oneValueArg MODULE_NAME FILE_DESCRIPTION ICON_FILE)
    set(multiValueArgs)
    cmake_parse_arguments(MODULE "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    message(STATUS "Parse Args Results:")
    message(STATUS "MODULE_NAME: ${MODULE_MODULE_NAME}")
    message(STATUS "FILE_DESCRIPTION: ${MODULE_FILE_DESCRIPTION}")
    message(STATUS "ICON_FILE: ${MODULE_ICON_FILE}")

    message(STATUS "***create .desktop file for: ${MODULE_MODULE_NAME}***")

    set(DESKTOP_FILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/${MODULE_MODULE_NAME}.desktop)
    generate_app_info_files(
        INPUT_JSON_FILE ${GLOBAL_APP_VERSION_JSON}
        INPUT_JSON_TARGET ${GLOBAL_APP_VERSION_JSON_TARGET}
        INPUT_VERSION_TEMPLATE ${GLOBAL_APP_DESKTOP_TEMPLATE}
        OUTPUT_FILE ${DESKTOP_FILE_PATH}
        INTERNAL_NAME ${MODULE_MODULE_NAME}
        FILE_DESCRIPTION "${MODULE_FILE_DESCRIPTION}"
        ORIGINAL_FILENAME ${MODULE_MODULE_NAME}
        OUTPUT_TARGET_VAR DESKTOP_FILE_TARGET
    )
    add_dependencies(${MODULE_MODULE_NAME} ${DESKTOP_FILE_TARGET})

    # Install .desktop and icon relative to CMAKE_INSTALL_PREFIX.
    # For local dev installs this goes to <prefix>/share/applications/.
    # For DEB packaging, cpack_pre_build.cmake relocates them to /usr/share/
    # so GNOME/KDE can discover them regardless of CPACK_PACKAGING_INSTALL_PREFIX.
    install(FILES ${DESKTOP_FILE_PATH}
        DESTINATION share/applications
    )

    # Install icon to standard hicolor theme location
    if(MODULE_ICON_FILE AND EXISTS "${MODULE_ICON_FILE}")
        install(FILES ${MODULE_ICON_FILE}
            DESTINATION share/icons/hicolor/256x256/apps
            RENAME "${MODULE_MODULE_NAME}.png"
        )
        message(STATUS "  Icon: ${MODULE_ICON_FILE} -> ${MODULE_MODULE_NAME}.png")
    else()
        message(WARNING "  Icon not found: ${MODULE_ICON_FILE}")
    endif()
endfunction()
