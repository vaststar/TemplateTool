
# Auto-generate CPack config from product_info.json (all platforms)
# Config is read by cpack at packaging time via CPACK_PROJECT_CONFIG_FILE
include(BuildCPackModule)
set(CPACK_TEMPLATES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/templates")
set(CPACK_PRE_BUILD "${CMAKE_CURRENT_SOURCE_DIR}/cpack_pre_build.cmake")

if(WIN32)
    BuildCPackModule(
        TEMPLATE "${CPACK_TEMPLATES_DIR}/cpack_windows_config.cmake.in"
        PRE_BUILD_SCRIPT "${CPACK_PRE_BUILD}"
        OUTPUT_CONFIG_VAR PLATFORM_CPACK_CONFIG
    )
elseif(APPLE)
    BuildCPackModule(
        TEMPLATE "${CPACK_TEMPLATES_DIR}/cpack_macos_config.cmake.in"
        PRE_BUILD_SCRIPT "${CPACK_PRE_BUILD}"
        OUTPUT_CONFIG_VAR PLATFORM_CPACK_CONFIG
    )
elseif(UNIX)
    BuildCPackModule(
        TEMPLATE "${CPACK_TEMPLATES_DIR}/cpack_linux_config.cmake.in"
        PRE_BUILD_SCRIPT "${CPACK_PRE_BUILD}"
        OUTPUT_CONFIG_VAR PLATFORM_CPACK_CONFIG
    )
    # Install to private /opt directory to avoid polluting /usr/bin with
    # plugins, qml files, etc.  .desktop and icons are relocated to
    # /usr/share/ by cpack_pre_build.cmake during packaging.
    set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/template-factory")
endif()

# Must be last — CPack reads all CPACK_* variables at this point
include(CPack)
