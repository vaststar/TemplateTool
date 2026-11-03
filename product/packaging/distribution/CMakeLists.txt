
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

# Runtime deployment keeps its bin/lib layout; only CPack's private staging
# tree is flattened and stripped of developer artifacts.
set(CPACK_RUNTIME_PRE_BUILD_SCRIPT
    "${CMAKE_CURRENT_BINARY_DIR}/cpack_runtime_pre_build.cmake")

# Include platform setup in this directory's scope so generated configuration
# and release metadata retain their existing build-tree paths.
if(WIN32)
    include("${CMAKE_CURRENT_SOURCE_DIR}/windows/packaging.cmake")
elseif(APPLE)
    include("${CMAKE_CURRENT_SOURCE_DIR}/macos/packaging.cmake")
elseif(UNIX)
    include("${CMAKE_CURRENT_SOURCE_DIR}/linux/packaging.cmake")
endif()

# Platform setup must precede this substitution: the Linux staging script
# embeds CPACK_LINUX_PRE_BUILD_SCRIPT in the common runtime preparation hook.
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/common/cpack_runtime_pre_build.cmake.in"
    "${CPACK_RUNTIME_PRE_BUILD_SCRIPT}"
    @ONLY
)
BuildCPackModule(
    TEMPLATE "${TT_PLATFORM_CPACK_TEMPLATE}"
    PRE_BUILD_SCRIPT "${CPACK_RUNTIME_PRE_BUILD_SCRIPT}"
    OUTPUT_CONFIG_VAR PLATFORM_CPACK_CONFIG
)

# ── Generate package-info.json (used by CI to build upgrade-manifest.json) ──
generate_app_info_files(
    INPUT_JSON_FILE ${APP_VERSION_JSON}
    INPUT_JSON_TARGET ${APP_VERSION_TARGET}
    INPUT_VERSION_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/common/package-info.json.in"
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
