include(GenerateAppVersionMeta)

set(GLOBAL_APP_VERSION_JSON ${CMAKE_CURRENT_BINARY_DIR}/version_meta.json CACHE INTERNAL "Global app json version filePath")
generate_app_version_meta(
    INPUT_META_JSON ${CMAKE_CURRENT_SOURCE_DIR}/product_info.json
    INPUT_VERSION_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/templates/version_meta.json.in
    OUTPUT_FILE ${GLOBAL_APP_VERSION_JSON}
    VERSION_META_TARGET
)
set(GLOBAL_APP_VERSION_JSON_TARGET ${VERSION_META_TARGET} CACHE INTERNAL "Global app json version target")

if (APPLE)
    set(GLOBAL_APP_RC_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/templates/app_mac_info.plist.in CACHE INTERNAL "Global app app_mac_info.plist.in template filePath")
else()
    set(GLOBAL_APP_RC_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/templates/app_resource.rc.in CACHE INTERNAL "Global app_resource.rc.in template filePath")
endif()
