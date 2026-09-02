include_guard()

include(GenerateAppInfoFiles)
include(GetRequiredTargetProperty)

function(BuildBundlePListModule)
    set(one_value_args MODULE_NAME FILE_DESCRIPTION)
    cmake_parse_arguments(PARSE_ARGV 0 MODULE "" "${one_value_args}" "")

    if(NOT MODULE_MODULE_NAME)
        message(FATAL_ERROR "[BuildBundlePListModule] MODULE_NAME is required")
    endif()
    if(NOT TARGET "${MODULE_MODULE_NAME}")
        message(FATAL_ERROR
            "[BuildBundlePListModule] Target does not exist: "
            "${MODULE_MODULE_NAME}")
    endif()
    if(NOT MODULE_FILE_DESCRIPTION)
        message(FATAL_ERROR
            "[BuildBundlePListModule] FILE_DESCRIPTION is required for "
            "${MODULE_MODULE_NAME}")
    endif()
    if(MODULE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[BuildBundlePListModule] Unknown arguments: "
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
        PROPERTY UCF_APP_MACOS_PLIST_TEMPLATE
        OUTPUT_VARIABLE app_plist_template
    )
    if(NOT TARGET "${app_version_target}")
        message(FATAL_ERROR
            "[BuildBundlePListModule] AppVersionMetadata generator target does "
            "not exist: ${app_version_target}")
    endif()
    if(NOT EXISTS "${app_plist_template}")
        message(FATAL_ERROR
            "[BuildBundlePListModule] plist template does not exist: "
            "${app_plist_template}")
    endif()

    set(plist_path
        "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_MODULE_NAME}_Info.plist")
    set(plist_placeholder_path
        "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_MODULE_NAME}_Info.plist.in")

    message(STATUS
        "[BuildBundlePListModule] ${MODULE_MODULE_NAME} -> ${plist_path}")
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS
            "[BuildBundlePListModule]   Description: ${MODULE_FILE_DESCRIPTION}")
        message(STATUS
            "[BuildBundlePListModule]   Template   : ${app_plist_template}")
    endif()

    # CMake requires its plist template to exist during generation. Keep that
    # placeholder separate from the build-time output so it cannot make the
    # real generation command appear up to date on a clean build.
    if(NOT EXISTS "${plist_placeholder_path}")
        file(WRITE "${plist_placeholder_path}"
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<plist version=\"1.0\"><dict/></plist>\n")
    endif()

    set_target_properties("${MODULE_MODULE_NAME}" PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${plist_placeholder_path}"
    )

    generate_app_info_files(
        INPUT_JSON_FILE "${app_version_json}"
        INPUT_JSON_TARGET "${app_version_target}"
        INPUT_VERSION_TEMPLATE "${app_plist_template}"
        OUTPUT_FILE "${plist_path}"
        INTERNAL_NAME "${MODULE_MODULE_NAME}"
        FILE_DESCRIPTION "${MODULE_FILE_DESCRIPTION}"
        ORIGINAL_FILENAME "$<TARGET_FILE_NAME:${MODULE_MODULE_NAME}>"
        OUTPUT_TARGET_VAR plist_target
    )

    set(plist_sync_target "${MODULE_MODULE_NAME}_sync_bundle_plist")
    if(TARGET "${plist_sync_target}")
        message(FATAL_ERROR "[BuildBundlePListModule] Target already exists: ${plist_sync_target}")
    endif()

    add_custom_target("${plist_sync_target}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_BUNDLE_DIR:${MODULE_MODULE_NAME}>/Contents"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${plist_path}" "$<TARGET_BUNDLE_DIR:${MODULE_MODULE_NAME}>/Contents/Info.plist"
        DEPENDS "${plist_path}"
        COMMENT "Synchronizing generated Info.plist into app bundle"
        VERBATIM
    )
    add_dependencies("${plist_sync_target}" "${plist_target}")
    add_dependencies("${MODULE_MODULE_NAME}" "${plist_sync_target}")
    set_target_properties("${plist_sync_target}" PROPERTIES FOLDER codegen)
endfunction()
