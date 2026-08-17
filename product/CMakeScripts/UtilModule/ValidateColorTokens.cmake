include_guard()

include("${CMAKE_CURRENT_LIST_DIR}/internal/TokenValidationInternals.cmake")

function(validate_color_tokens)
    set(one_value_args
        TOKEN_FILE
        PALETTE_FILE
        SCRIPT
        OUTPUT_TARGET_VAR
    )
    set(multi_value_args THEME_FILES)
    cmake_parse_arguments(
        PARSE_ARGV 0 VCT "" "${one_value_args}" "${multi_value_args}")

    foreach(required_arg TOKEN_FILE SCRIPT OUTPUT_TARGET_VAR)
        if(NOT VCT_${required_arg})
            message(FATAL_ERROR
                "[validate_color_tokens] ${required_arg} is required")
        endif()
    endforeach()
    if(NOT VCT_THEME_FILES)
        message(FATAL_ERROR
            "[validate_color_tokens] THEME_FILES is required")
    endif()
    if(VCT_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[validate_color_tokens] Arguments missing values: "
            "${VCT_KEYWORDS_MISSING_VALUES}")
    endif()
    if(VCT_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[validate_color_tokens] Unknown arguments: "
            "${VCT_UNPARSED_ARGUMENTS}")
    endif()
    _tt_token_validation_validate_output_variable(
        FUNCTION_NAME validate_color_tokens
        VARIABLE_NAME "${VCT_OUTPUT_TARGET_VAR}"
    )

    set(optional_validation_args)
    if(VCT_PALETTE_FILE)
        _tt_token_validation_normalize_path(
            PATH "${VCT_PALETTE_FILE}"
            BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE palette_file
        )
        list(APPEND optional_validation_args
            ADDITIONAL_DEPENDS "${palette_file}"
            EXTRA_ARGUMENTS --palette "${palette_file}"
        )
    endif()

    _tt_register_token_validation(
        KIND color
        SCRIPT "${VCT_SCRIPT}"
        TOKEN_FILE "${VCT_TOKEN_FILE}"
        THEME_FILES ${VCT_THEME_FILES}
        ${optional_validation_args}
        COMMENT "Validating color token consistency"
        OUTPUT_TARGET_VAR validation_target
    )

    set("${VCT_OUTPUT_TARGET_VAR}" "${validation_target}" PARENT_SCOPE)
endfunction()
