
# Remove development files (include/, cmake/) before packaging
set(CPACK_PRE_BUILD_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/cpack_pre_build.cmake")

# Auto-generate CPack config from product_info.json (all platforms)
include(BuildCPackModule)
set(CPACK_TEMPLATES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/templates")

if(WIN32)
    BuildCPackModule(
        TEMPLATE "${CPACK_TEMPLATES_DIR}/cpack_windows_config.cmake.in"
        OUTPUT_CONFIG_VAR PLATFORM_CPACK_CONFIG
    )
elseif(APPLE)
    BuildCPackModule(
        TEMPLATE "${CPACK_TEMPLATES_DIR}/cpack_macos_config.cmake.in"
        OUTPUT_CONFIG_VAR PLATFORM_CPACK_CONFIG
    )
elseif(UNIX)
    BuildCPackModule(
        TEMPLATE "${CPACK_TEMPLATES_DIR}/cpack_linux_config.cmake.in"
        OUTPUT_CONFIG_VAR PLATFORM_CPACK_CONFIG
    )
endif()

if(PLATFORM_CPACK_CONFIG AND EXISTS "${PLATFORM_CPACK_CONFIG}")
    include(${PLATFORM_CPACK_CONFIG})
else()
    # Fallback defaults
    set(CPACK_PACKAGE_NAME "mainEntry")
    set(CPACK_PACKAGE_VERSION "0.0.0")
    set(CPACK_PACKAGE_CONTACT "47029316@qq.com")
    set(CPACK_GENERATOR "TGZ")
endif()

# include CPack 必须在最后
include(CPack)
