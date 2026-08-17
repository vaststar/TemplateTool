include(GenerateAppVersionMeta)

set(APP_VERSION_JSON ${CMAKE_CURRENT_BINARY_DIR}/version_meta.json)
generate_app_version_meta(
    INPUT_META_JSON ${CMAKE_CURRENT_SOURCE_DIR}/product_info.json
    INPUT_VERSION_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/templates/version_meta.json.in
    OUTPUT_FILE ${APP_VERSION_JSON}
    OUTPUT_TARGET_VAR VERSION_META_TARGET
)

add_library(AppVersionMetadata INTERFACE)
set_property(
    TARGET AppVersionMetadata
    PROPERTY UCF_APP_VERSION_JSON_FILE "${APP_VERSION_JSON}"
)
set_property(
    TARGET AppVersionMetadata
    PROPERTY UCF_APP_VERSION_GENERATOR_TARGET "${VERSION_META_TARGET}"
)
set_property(
    TARGET AppVersionMetadata
    PROPERTY UCF_APP_MACOS_PLIST_TEMPLATE
        "${CMAKE_CURRENT_SOURCE_DIR}/templates/app_mac_info.plist.in"
)
set_property(
    TARGET AppVersionMetadata
    PROPERTY UCF_APP_WINDOWS_RC_TEMPLATE
        "${CMAKE_CURRENT_SOURCE_DIR}/templates/app_resource.rc.in"
)
set_property(
    TARGET AppVersionMetadata
    PROPERTY UCF_APP_LINUX_DESKTOP_TEMPLATE
        "${CMAKE_CURRENT_SOURCE_DIR}/templates/app_linux.desktop.in"
)
