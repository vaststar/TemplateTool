include_guard()

include(GenerateAppInfoFiles)
include(GetRequiredTargetProperty)

function(BuildRCFileModule)
    set(one_value_args MODULE_NAME FILE_DESCRIPTION)
    cmake_parse_arguments(PARSE_ARGV 0 MODULE "" "${one_value_args}" "")

    if(NOT MODULE_MODULE_NAME)
        message(FATAL_ERROR "[BuildRCFileModule] MODULE_NAME is required")
    endif()
    if(NOT TARGET "${MODULE_MODULE_NAME}")
        message(FATAL_ERROR
            "[BuildRCFileModule] Target does not exist: ${MODULE_MODULE_NAME}")
    endif()
    if(NOT MODULE_FILE_DESCRIPTION)
        message(FATAL_ERROR
            "[BuildRCFileModule] FILE_DESCRIPTION is required for "
            "${MODULE_MODULE_NAME}")
    endif()
    if(MODULE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[BuildRCFileModule] Unknown arguments: ${MODULE_UNPARSED_ARGUMENTS}")
    endif()

    get_target_property(target_type "${MODULE_MODULE_NAME}" TYPE)
    if(target_type STREQUAL "EXECUTABLE")
        set(file_type VFT_APP)
    elseif(target_type STREQUAL "SHARED_LIBRARY" OR target_type STREQUAL "MODULE_LIBRARY")
        set(file_type VFT_DLL)
    else()
        message(FATAL_ERROR "[BuildRCFileModule] Unsupported target type '${target_type}' for '${MODULE_MODULE_NAME}'")
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
        PROPERTY UCF_APP_WINDOWS_RC_TEMPLATE
        OUTPUT_VARIABLE app_rc_template
    )
    if(NOT TARGET "${app_version_target}")
        message(FATAL_ERROR
            "[BuildRCFileModule] AppVersionMetadata generator target does not "
            "exist: ${app_version_target}")
    endif()
    if(NOT EXISTS "${app_rc_template}")
        message(FATAL_ERROR
            "[BuildRCFileModule] RC template does not exist: "
            "${app_rc_template}")
    endif()

    set(app_rc_path
        "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_MODULE_NAME}_resource.rc")

    message(STATUS
        "[BuildRCFileModule] ${MODULE_MODULE_NAME} -> ${app_rc_path}")
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS
            "[BuildRCFileModule]   Description: ${MODULE_FILE_DESCRIPTION}")
        message(STATUS
            "[BuildRCFileModule]   Template   : ${app_rc_template}")
    endif()

    generate_app_info_files(
        INPUT_JSON_FILE "${app_version_json}"
        INPUT_JSON_TARGET "${app_version_target}"
        INPUT_VERSION_TEMPLATE "${app_rc_template}"
        OUTPUT_FILE "${app_rc_path}"
        INTERNAL_NAME "${MODULE_MODULE_NAME}"
        FILE_DESCRIPTION "${MODULE_FILE_DESCRIPTION}"
        ORIGINAL_FILENAME "$<TARGET_FILE_NAME:${MODULE_MODULE_NAME}>"
        FILE_TYPE "${file_type}"
        OUTPUT_TARGET_VAR app_rc_target
    )

    target_sources("${MODULE_MODULE_NAME}" PRIVATE "${app_rc_path}")
    add_dependencies("${MODULE_MODULE_NAME}" "${app_rc_target}")
endfunction()
