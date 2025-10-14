include(GenerateAppVersionMeta)

set(GLOBAL_APP_VERSION_JSON ${UCF_CODE_GENERATOR_DIR}/AppVersion/version_meta.json CACHE INTERNAL "Global app json version filePath")
generate_app_version_meta(
    INPUT_META_JSON ${CMAKE_CURRENT_SOURCE_DIR}/product_info.json
    INPUT_VERSION_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/templates/version_meta.json.in
    OUTPUT_FILE ${GLOBAL_APP_VERSION_JSON}
    VERSION_META_TARGET
)
set(GLOBAL_APP_VERSION_JSON_TARGET ${VERSION_META_TARGET} CACHE INTERNAL "Global app json version target")
set(GLOBAL_APP_RC_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/templates/app_resource.rc.in CACHE INTERNAL "Global app rc template filePath")
