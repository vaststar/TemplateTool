
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
        "[distribution] AppVersionMetadata generator target does not exist: "
        "${APP_VERSION_TARGET}")
endif()

set(CPACK_TEMPLATES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/templates")

if(UNIX AND NOT APPLE)
    set(CPACK_LINUX_PRE_BUILD_SCRIPT
        "${CMAKE_CURRENT_BINARY_DIR}/cpack_linux_pre_build.cmake")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cpack_linux_pre_build.cmake.in"
        "${CPACK_LINUX_PRE_BUILD_SCRIPT}"
        @ONLY
    )
endif()

# Runtime deployment keeps its bin/lib layout; only CPack's private staging
# tree is flattened and stripped of developer artifacts.
set(CPACK_RUNTIME_PRE_BUILD_SCRIPT
    "${CMAKE_CURRENT_BINARY_DIR}/cpack_runtime_pre_build.cmake")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cpack_runtime_pre_build.cmake.in"
    "${CPACK_RUNTIME_PRE_BUILD_SCRIPT}"
    @ONLY
)

if(WIN32)
    set(CPACK_NSIS_POLICY_FILE
        "${CMAKE_CURRENT_SOURCE_DIR}/nsis_install_policy.nsh")
    BuildCPackModule(
        TEMPLATE "${CPACK_TEMPLATES_DIR}/cpack_windows_config.cmake.in"
        PRE_BUILD_SCRIPT "${CPACK_RUNTIME_PRE_BUILD_SCRIPT}"
        OUTPUT_CONFIG_VAR PLATFORM_CPACK_CONFIG
    )
elseif(APPLE)
    BuildCPackModule(
        TEMPLATE "${CPACK_TEMPLATES_DIR}/cpack_macos_config.cmake.in"
        PRE_BUILD_SCRIPT "${CPACK_RUNTIME_PRE_BUILD_SCRIPT}"
        OUTPUT_CONFIG_VAR PLATFORM_CPACK_CONFIG
    )
elseif(UNIX)
    BuildCPackModule(
        TEMPLATE "${CPACK_TEMPLATES_DIR}/cpack_linux_config.cmake.in"
        PRE_BUILD_SCRIPT "${CPACK_RUNTIME_PRE_BUILD_SCRIPT}"
        OUTPUT_CONFIG_VAR PLATFORM_CPACK_CONFIG
    )
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

# Package one runtime tree. Component installers record their file lists before
# the pre-build script moves files out of bin/.
set(CPACK_MONOLITHIC_INSTALL ON)
set(CPACK_APPLICATION_EXECUTABLE_NAME "${TT_APPLICATION_EXECUTABLE_NAME}")

# Must be last — CPack reads all CPACK_* variables at this point
include(CPack)
