include_guard()

function(_tt_version_normalize_path)
    set(one_value_args PATH BASE_DIRECTORY OUTPUT_VARIABLE)
    cmake_parse_arguments(
        PARSE_ARGV 0 ARG "" "${one_value_args}" "")

    foreach(required_arg PATH BASE_DIRECTORY OUTPUT_VARIABLE)
        if(NOT ARG_${required_arg})
            message(FATAL_ERROR
                "[VersionGeneration] ${required_arg} is required")
        endif()
    endforeach()
    if(ARG_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[VersionGeneration] Arguments missing values: "
            "${ARG_KEYWORDS_MISSING_VALUES}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[VersionGeneration] Unknown arguments: "
            "${ARG_UNPARSED_ARGUMENTS}")
    endif()

    set(normalized_path "${ARG_PATH}")
    if(IS_ABSOLUTE "${normalized_path}")
        cmake_path(NORMAL_PATH normalized_path)
    else()
        cmake_path(
            ABSOLUTE_PATH normalized_path
            BASE_DIRECTORY "${ARG_BASE_DIRECTORY}"
            NORMALIZE
        )
    endif()

    set("${ARG_OUTPUT_VARIABLE}" "${normalized_path}" PARENT_SCOPE)
endfunction()

function(_tt_version_validate_output_variable)
    set(one_value_args FUNCTION_NAME VARIABLE_NAME)
    cmake_parse_arguments(
        PARSE_ARGV 0 ARG "" "${one_value_args}" "")

    foreach(required_arg FUNCTION_NAME VARIABLE_NAME)
        if(NOT ARG_${required_arg})
            message(FATAL_ERROR
                "[VersionGeneration] ${required_arg} is required")
        endif()
    endforeach()
    if(NOT "${ARG_VARIABLE_NAME}" MATCHES "^[A-Za-z_][A-Za-z0-9_]*$")
        message(FATAL_ERROR
            "[${ARG_FUNCTION_NAME}] OUTPUT_TARGET_VAR must be a valid "
            "variable name, got '${ARG_VARIABLE_NAME}'")
    endif()
endfunction()

function(_tt_version_make_target_name)
    set(one_value_args PREFIX OUTPUT_FILE OUTPUT_VARIABLE)
    cmake_parse_arguments(
        PARSE_ARGV 0 ARG "" "${one_value_args}" "")

    foreach(required_arg PREFIX OUTPUT_FILE OUTPUT_VARIABLE)
        if(NOT ARG_${required_arg})
            message(FATAL_ERROR
                "[VersionGeneration] ${required_arg} is required")
        endif()
    endforeach()

    get_filename_component(output_name "${ARG_OUTPUT_FILE}" NAME)
    string(MAKE_C_IDENTIFIER "${output_name}" safe_output_name)
    string(SHA256 output_hash "${ARG_OUTPUT_FILE}")
    string(SUBSTRING "${output_hash}" 0 12 short_output_hash)

    set("${ARG_OUTPUT_VARIABLE}"
        "${ARG_PREFIX}_${safe_output_name}_${short_output_hash}"
        PARENT_SCOPE
    )
endfunction()

function(_tt_version_check_registration)
    set(one_value_args TARGET OUTPUT_FILE SIGNATURE OUTPUT_REUSED_VAR)
    cmake_parse_arguments(
        PARSE_ARGV 0 ARG "" "${one_value_args}" "")

    foreach(required_arg TARGET OUTPUT_FILE SIGNATURE OUTPUT_REUSED_VAR)
        if(NOT ARG_${required_arg})
            message(FATAL_ERROR
                "[VersionGeneration] ${required_arg} is required")
        endif()
    endforeach()

    set(reused FALSE)
    if(TARGET "${ARG_TARGET}")
        get_target_property(
            registered_output
            "${ARG_TARGET}"
            TT_VERSION_GENERATED_OUTPUT
        )
        get_target_property(
            registered_signature
            "${ARG_TARGET}"
            TT_VERSION_GENERATOR_SIGNATURE
        )

        if(NOT "${registered_output}" STREQUAL "${ARG_OUTPUT_FILE}")
            message(FATAL_ERROR
                "[VersionGeneration] Target-name collision for "
                "'${ARG_TARGET}': '${registered_output}' and "
                "'${ARG_OUTPUT_FILE}'")
        endif()
        if(NOT "${registered_signature}" STREQUAL "${ARG_SIGNATURE}")
            message(FATAL_ERROR
                "[VersionGeneration] Output '${ARG_OUTPUT_FILE}' is already "
                "registered with a different generation configuration")
        endif()

        set(reused TRUE)
    endif()

    set("${ARG_OUTPUT_REUSED_VAR}" "${reused}" PARENT_SCOPE)
endfunction()

function(_tt_version_record_registration)
    set(one_value_args TARGET OUTPUT_FILE SIGNATURE)
    cmake_parse_arguments(
        PARSE_ARGV 0 ARG "" "${one_value_args}" "")

    foreach(required_arg TARGET OUTPUT_FILE SIGNATURE)
        if(NOT ARG_${required_arg})
            message(FATAL_ERROR
                "[VersionGeneration] ${required_arg} is required")
        endif()
    endforeach()
    if(NOT TARGET "${ARG_TARGET}")
        message(FATAL_ERROR
            "[VersionGeneration] Cannot register missing target: "
            "${ARG_TARGET}")
    endif()

    set_target_properties("${ARG_TARGET}" PROPERTIES
        FOLDER codegen
        TT_VERSION_GENERATED_OUTPUT "${ARG_OUTPUT_FILE}"
        TT_VERSION_GENERATOR_SIGNATURE "${ARG_SIGNATURE}"
    )
endfunction()
