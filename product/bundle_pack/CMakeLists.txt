
# Auto-generate CPack config from product_info.json (all platforms)
# Config is read by cpack at packaging time via CPACK_PROJECT_CONFIG_FILE
include(BuildCPackModule)
include(GetRequiredTargetProperty)

get_required_target_property(
    TARGET AppVersionMetadata
    PROPERTY UCF_APP_VERSION_JSON_FILE
    OUTPUT_VARIABLE APP_VERSION_JSON
)
get_required_target_property(
    TARGET AppVersionMetadata
    PROPERTY UCF_APP_VERSION_GENERATOR_TARGET
    OUTPUT_VARIABLE APP_VERSION_TARGET
)
if(NOT TARGET "${APP_VERSION_TARGET}")
    message(FATAL_ERROR
        "[bundle_pack] AppVersionMetadata generator target does not exist: "
        "${APP_VERSION_TARGET}")
endif()

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
        PRE_BUILD_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cpack_linux_pre_build.cmake"
        OUTPUT_CONFIG_VAR PLATFORM_CPACK_CONFIG
    )
    set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/template-factory")
endif()

# ── Generate package-info.json (used by CI to build upgrade-manifest.json) ──
generate_app_info_files(
    INPUT_JSON_FILE ${APP_VERSION_JSON}
    INPUT_JSON_TARGET ${APP_VERSION_TARGET}
    INPUT_VERSION_TEMPLATE "${CPACK_TEMPLATES_DIR}/package-info.json.in"
    OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/package-info.json"
    INTERNAL_NAME ""
    FILE_DESCRIPTION ""
    ORIGINAL_FILENAME ""
    OUTPUT_TARGET_VAR PACKAGE_INFO_TARGET
)

# Must be last — CPack reads all CPACK_* variables at this point
include(CPack)
