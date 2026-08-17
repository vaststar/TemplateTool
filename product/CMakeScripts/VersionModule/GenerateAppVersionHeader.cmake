include_guard()

include("${CMAKE_CURRENT_LIST_DIR}/GenerateAppInfoFiles.cmake")

function(generate_app_version_header)
    set(one_value_args
        INPUT_JSON_FILE
        INPUT_JSON_TARGET
        INPUT_VERSION_TEMPLATE
        OUTPUT_FILE
        OUTPUT_TARGET_VAR
    )
    cmake_parse_arguments(
        PARSE_ARGV 0 ARG "" "${one_value_args}" "")

    foreach(required_arg IN LISTS one_value_args)
        if(NOT ARG_${required_arg})
            message(FATAL_ERROR
                "[generate_app_version_header] ${required_arg} is required")
        endif()
    endforeach()
    if(ARG_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[generate_app_version_header] Arguments missing values: "
            "${ARG_KEYWORDS_MISSING_VALUES}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[generate_app_version_header] Unknown arguments: "
            "${ARG_UNPARSED_ARGUMENTS}")
    endif()

    generate_app_info_files(
        INPUT_JSON_FILE "${ARG_INPUT_JSON_FILE}"
        INPUT_JSON_TARGET "${ARG_INPUT_JSON_TARGET}"
        INPUT_VERSION_TEMPLATE "${ARG_INPUT_VERSION_TEMPLATE}"
        OUTPUT_FILE "${ARG_OUTPUT_FILE}"
        INTERNAL_NAME ""
        FILE_DESCRIPTION ""
        ORIGINAL_FILENAME ""
        OUTPUT_TARGET_VAR generated_target
    )

    set("${ARG_OUTPUT_TARGET_VAR}" "${generated_target}" PARENT_SCOPE)
endfunction()
