include_guard()

if(CMAKE_SCRIPT_MODE_FILE)
    foreach(required_variable INPUT_JSON_FILE INPUT_TEMPLATE_FILE OUTPUT_H)
        if(NOT DEFINED ${required_variable}
           OR "${${required_variable}}" STREQUAL "")
            message(FATAL_ERROR
                "[GenerateAppInfoFiles] ${required_variable} is required")
        endif()
    endforeach()

    foreach(required_file "${INPUT_JSON_FILE}" "${INPUT_TEMPLATE_FILE}")
        if(NOT EXISTS "${required_file}")
            message(FATAL_ERROR
                "[GenerateAppInfoFiles] Input file does not exist: "
                "${required_file}")
        endif()
    endforeach()

    file(READ "${INPUT_JSON_FILE}" json_content)
    string(JSON VERSION_MAJOR GET
        "${json_content}" "VERSION" "VERSION_MAJOR")
    string(JSON VERSION_MINOR GET
        "${json_content}" "VERSION" "VERSION_MINOR")
    string(JSON VERSION_PATCH GET
        "${json_content}" "VERSION" "VERSION_PATCH")
    string(JSON VERSION_BUILD GET
        "${json_content}" "VERSION" "VERSION_BUILD")
    string(JSON GIT_COMMIT_HASH GET
        "${json_content}" "COMPILATION" "GIT_COMMIT_HASH")
    string(JSON GIT_COMMIT_BRANCH GET
        "${json_content}" "COMPILATION" "GIT_COMMIT_BRANCH")
    string(JSON COMPANY_NAME GET "${json_content}" "COMPANY" "NAME")
    string(JSON COPYRIGHT GET "${json_content}" "COMPANY" "COPYRIGHT")
    string(JSON PRODUCT_NAME GET "${json_content}" "PRODUCT" "NAME")
    string(JSON PRODUCT_IDENTIFIER GET "${json_content}" "PRODUCT" "IDENTIFIER")
    string(JSON PRODUCT_DESCRIPTION GET
        "${json_content}" "PRODUCT" "DESCRIPTION")

    foreach(version_component VERSION_MAJOR VERSION_MINOR VERSION_PATCH VERSION_BUILD)
        if(NOT "${${version_component}}" MATCHES "^[0-9]+$")
            message(FATAL_ERROR "[GenerateAppInfoFiles] ${version_component} must contain only digits")
        endif()

        string(REGEX REPLACE "^0+" "" normalized_component "${${version_component}}")
        if(normalized_component STREQUAL "")
            set(normalized_component 0)
        endif()
        if(FILE_TYPE AND normalized_component GREATER 65535)
            message(FATAL_ERROR "[GenerateAppInfoFiles] ${version_component} exceeds the Windows version component limit")
        endif()

        set("${version_component}_NUMERIC" "${normalized_component}")
    endforeach()

    set(FILE_VERSION "${VERSION_MAJOR_NUMERIC},${VERSION_MINOR_NUMERIC},${VERSION_PATCH_NUMERIC},${VERSION_BUILD_NUMERIC}")
    set(PRODUCT_VERSION "${FILE_VERSION}")
    set(FILE_VERSION_STR
        "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_BUILD}")
    set(PRODUCT_VERSION_STR "${FILE_VERSION_STR}")
    set(MACOS_BUNDLE_VERSION "${VERSION_BUILD}")
    if(MACOS_BUNDLE_VERSION STREQUAL "0")
        set(MACOS_BUNDLE_VERSION "1")
    endif()

    get_filename_component(output_directory "${OUTPUT_H}" DIRECTORY)
    file(MAKE_DIRECTORY "${output_directory}")
    configure_file("${INPUT_TEMPLATE_FILE}" "${OUTPUT_H}" @ONLY)
    message(STATUS "[GenerateAppInfoFiles] Generated ${OUTPUT_H}")
else()
    include(
        "${CMAKE_CURRENT_LIST_DIR}/internal/VersionGenerationInternals.cmake")

    function(generate_app_info_files)
        set(one_value_args
            INPUT_JSON_FILE
            INPUT_JSON_TARGET
            INPUT_VERSION_TEMPLATE
            OUTPUT_FILE
            INTERNAL_NAME
            FILE_DESCRIPTION
            ORIGINAL_FILENAME
            FILE_TYPE
            OUTPUT_TARGET_VAR
        )
        cmake_parse_arguments(
            PARSE_ARGV 0 ARG "" "${one_value_args}" "")

        foreach(required_arg
                INPUT_JSON_FILE
                INPUT_JSON_TARGET
                INPUT_VERSION_TEMPLATE
                OUTPUT_FILE
                OUTPUT_TARGET_VAR)
            if(NOT ARG_${required_arg})
                message(FATAL_ERROR
                    "[generate_app_info_files] ${required_arg} is required")
            endif()
        endforeach()
        if(ARG_KEYWORDS_MISSING_VALUES)
            message(FATAL_ERROR
                "[generate_app_info_files] Arguments missing values: "
                "${ARG_KEYWORDS_MISSING_VALUES}")
        endif()
        if(ARG_UNPARSED_ARGUMENTS)
            message(FATAL_ERROR
                "[generate_app_info_files] Unknown arguments: "
                "${ARG_UNPARSED_ARGUMENTS}")
        endif()
        if(NOT TARGET "${ARG_INPUT_JSON_TARGET}")
            message(FATAL_ERROR
                "[generate_app_info_files] INPUT_JSON_TARGET does not exist: "
                "${ARG_INPUT_JSON_TARGET}")
        endif()

        _tt_version_validate_output_variable(
            FUNCTION_NAME generate_app_info_files
            VARIABLE_NAME "${ARG_OUTPUT_TARGET_VAR}"
        )
        _tt_version_normalize_path(
            PATH "${ARG_INPUT_JSON_FILE}"
            BASE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            OUTPUT_VARIABLE input_json_file
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

        if(NOT EXISTS "${template_file}")
            message(FATAL_ERROR
                "[generate_app_info_files] Template does not exist: "
                "${template_file}")
        endif()

        _tt_version_make_target_name(
            PREFIX generate_app_info
            OUTPUT_FILE "${output_file}"
            OUTPUT_VARIABLE generated_target
        )

        set(generator_script "${CMAKE_CURRENT_FUNCTION_LIST_FILE}")
        string(CONCAT registration_data
            "input=${input_json_file}\n"
            "input_target=${ARG_INPUT_JSON_TARGET}\n"
            "template=${template_file}\n"
            "output=${output_file}\n"
            "internal_name=${ARG_INTERNAL_NAME}\n"
            "description=${ARG_FILE_DESCRIPTION}\n"
            "original_name=${ARG_ORIGINAL_FILENAME}\n"
            "file_type=${ARG_FILE_TYPE}\n"
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
                "-DINPUT_JSON_FILE=${input_json_file}"
                "-DINPUT_TEMPLATE_FILE=${template_file}"
                "-DFILE_DESCRIPTION=${ARG_FILE_DESCRIPTION}"
                "-DINTERNAL_NAME=${ARG_INTERNAL_NAME}"
                "-DORIGINAL_FILENAME=${ARG_ORIGINAL_FILENAME}"
                "-DFILE_TYPE=${ARG_FILE_TYPE}"
                "-DOUTPUT_H=${output_file}"
                -P "${generator_script}"
            DEPENDS
                "${ARG_INPUT_JSON_TARGET}"
                "${input_json_file}"
                "${template_file}"
                "${generator_script}"
            COMMENT "Generating ${output_file}"
            VERBATIM
        )

        add_custom_target(
            "${generated_target}" ALL DEPENDS "${output_file}")
        add_dependencies("${generated_target}" "${ARG_INPUT_JSON_TARGET}")
        _tt_version_record_registration(
            TARGET "${generated_target}"
            OUTPUT_FILE "${output_file}"
            SIGNATURE "${registration_signature}"
        )
        set_source_files_properties(
            "${output_file}" PROPERTIES GENERATED TRUE)

        message(STATUS
            "[generate_app_info_files] ${generated_target} -> ${output_file}")
        if(TT_CMAKE_VERBOSE_CONFIG)
            message(STATUS
                "[generate_app_info_files]   Input   : ${input_json_file}")
            message(STATUS
                "[generate_app_info_files]   Template: ${template_file}")
        endif()

        set("${ARG_OUTPUT_TARGET_VAR}"
            "${generated_target}"
            PARENT_SCOPE
        )
    endfunction()
endif()
