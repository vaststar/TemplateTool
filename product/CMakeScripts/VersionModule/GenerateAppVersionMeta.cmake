include_guard()

if(CMAKE_SCRIPT_MODE_FILE)
    foreach(required_variable
            INPUT_META_JSON
            INPUT_TEMPLATE
            OUTPUT_FILE
            GIT_INFO_FILE)
        if(NOT DEFINED ${required_variable}
           OR "${${required_variable}}" STREQUAL "")
            message(FATAL_ERROR
                "[GenerateAppVersionMeta] ${required_variable} is required")
        endif()
    endforeach()

    foreach(required_file
            "${INPUT_META_JSON}"
            "${INPUT_TEMPLATE}"
            "${GIT_INFO_FILE}")
        if(NOT EXISTS "${required_file}")
            message(FATAL_ERROR
                "[GenerateAppVersionMeta] Input file does not exist: "
                "${required_file}")
        endif()
    endforeach()

    file(READ "${GIT_INFO_FILE}" git_info_content)
    string(JSON GIT_COMMIT_HASH GET "${git_info_content}" "hash")
    string(JSON GIT_COMMIT_BRANCH GET "${git_info_content}" "branch")
    string(JSON VERSION_BUILD GET "${git_info_content}" "depth")

    file(READ "${INPUT_META_JSON}" json_content)
    string(JSON VERSION_MAJOR GET "${json_content}" "version" "major")
    string(JSON VERSION_MINOR GET "${json_content}" "version" "minor")
    string(JSON VERSION_PATCH GET "${json_content}" "version" "patch")
    string(JSON COMPANY_NAME GET "${json_content}" "company" "name")
    string(JSON COPYRIGHT GET "${json_content}" "company" "copyright")
    string(JSON PRODUCT_NAME GET "${json_content}" "product" "name")
    string(JSON PRODUCT_DESCRIPTION GET
        "${json_content}" "product" "description")

    get_filename_component(output_directory "${OUTPUT_FILE}" DIRECTORY)
    file(MAKE_DIRECTORY "${output_directory}")
    configure_file("${INPUT_TEMPLATE}" "${OUTPUT_FILE}" @ONLY)
    message(STATUS "[GenerateAppVersionMeta] Generated ${OUTPUT_FILE}")
else()
    include("${CMAKE_CURRENT_LIST_DIR}/GenerateGitInfoMeta.cmake")
    include(
        "${CMAKE_CURRENT_LIST_DIR}/internal/VersionGenerationInternals.cmake")

    function(generate_app_version_meta)
        set(one_value_args
            INPUT_META_JSON
            INPUT_VERSION_TEMPLATE
            OUTPUT_FILE
            OUTPUT_TARGET_VAR
        )
        cmake_parse_arguments(
            PARSE_ARGV 0 ARG "" "${one_value_args}" "")

        foreach(required_arg
                INPUT_META_JSON
                INPUT_VERSION_TEMPLATE
                OUTPUT_FILE
                OUTPUT_TARGET_VAR)
            if(NOT ARG_${required_arg})
                message(FATAL_ERROR
                    "[generate_app_version_meta] ${required_arg} is required")
            endif()
        endforeach()
        if(ARG_KEYWORDS_MISSING_VALUES)
            message(FATAL_ERROR
                "[generate_app_version_meta] Arguments missing values: "
                "${ARG_KEYWORDS_MISSING_VALUES}")
        endif()
        if(ARG_UNPARSED_ARGUMENTS)
            message(FATAL_ERROR
                "[generate_app_version_meta] Unknown arguments: "
                "${ARG_UNPARSED_ARGUMENTS}")
        endif()

        _tt_version_validate_output_variable(
            FUNCTION_NAME generate_app_version_meta
            VARIABLE_NAME "${ARG_OUTPUT_TARGET_VAR}"
        )
        _tt_version_normalize_path(
            PATH "${ARG_INPUT_META_JSON}"
            BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE input_meta_json
        )
        _tt_version_normalize_path(
            PATH "${ARG_INPUT_VERSION_TEMPLATE}"
            BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE template_file
        )
        _tt_version_normalize_path(
            PATH "${ARG_OUTPUT_FILE}"
            BASE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            OUTPUT_VARIABLE output_file
        )

        foreach(required_file "${input_meta_json}" "${template_file}")
            if(NOT EXISTS "${required_file}")
                message(FATAL_ERROR
                    "[generate_app_version_meta] Input file does not exist: "
                    "${required_file}")
            endif()
        endforeach()

        set(git_info_file "${CMAKE_CURRENT_BINARY_DIR}/git_info.meta")
        generate_git_info_meta(
            OUTPUT_FILE "${git_info_file}"
            OUTPUT_TARGET_VAR git_info_target
        )

        _tt_version_make_target_name(
            PREFIX generate_version_meta
            OUTPUT_FILE "${output_file}"
            OUTPUT_VARIABLE generated_target
        )

        set(generator_script "${CMAKE_CURRENT_FUNCTION_LIST_FILE}")
        string(CONCAT registration_data
            "input=${input_meta_json}\n"
            "template=${template_file}\n"
            "output=${output_file}\n"
            "git_file=${git_info_file}\n"
            "git_target=${git_info_target}\n"
            "script=${generator_script}\n"
        )
        string(SHA256 registration_signature "${registration_data}")

        _tt_version_check_registration(
            TARGET "${generated_target}"
            OUTPUT_FILE "${output_file}"
            SIGNATURE "${registration_signature}"
            OUTPUT_REUSED_VAR reused
        )
        if(reused)
            set("${ARG_OUTPUT_TARGET_VAR}"
                "${generated_target}"
                PARENT_SCOPE
            )
            return()
        endif()

        get_filename_component(output_directory "${output_file}" DIRECTORY)
        add_custom_command(
            OUTPUT "${output_file}"
            COMMAND
                "${CMAKE_COMMAND}" -E make_directory "${output_directory}"
            COMMAND
                "${CMAKE_COMMAND}"
                "-DINPUT_META_JSON=${input_meta_json}"
                "-DINPUT_TEMPLATE=${template_file}"
                "-DOUTPUT_FILE=${output_file}"
                "-DGIT_INFO_FILE=${git_info_file}"
                -P "${generator_script}"
            DEPENDS
                "${git_info_target}"
                "${git_info_file}"
                "${input_meta_json}"
                "${template_file}"
                "${generator_script}"
            COMMENT "Generating ${output_file}"
            VERBATIM
        )

        add_custom_target(
            "${generated_target}" ALL DEPENDS "${output_file}")
        add_dependencies("${generated_target}" "${git_info_target}")
        _tt_version_record_registration(
            TARGET "${generated_target}"
            OUTPUT_FILE "${output_file}"
            SIGNATURE "${registration_signature}"
        )
        set_source_files_properties(
            "${output_file}" PROPERTIES GENERATED TRUE)

        message(STATUS
            "[generate_app_version_meta] ${generated_target} -> ${output_file}")
        if(CMAKE_VERBOSE_MAKEFILE)
            message(STATUS
                "[generate_app_version_meta]   Input   : ${input_meta_json}")
            message(STATUS
                "[generate_app_version_meta]   Template: ${template_file}")
            message(STATUS
                "[generate_app_version_meta]   Git info: ${git_info_file}")
        endif()

        set("${ARG_OUTPUT_TARGET_VAR}"
            "${generated_target}"
            PARENT_SCOPE
        )
    endfunction()
endif()
