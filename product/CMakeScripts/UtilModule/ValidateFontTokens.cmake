include_guard()

include("${CMAKE_CURRENT_LIST_DIR}/internal/TokenValidationInternals.cmake")

function(validate_font_tokens)
    set(one_value_args
        TOKEN_FILE
        PALETTE_FILE
        SCRIPT
        OUTPUT_TARGET_VAR
    )
    set(multi_value_args THEME_FILES)
    cmake_parse_arguments(
        PARSE_ARGV 0 VFT "" "${one_value_args}" "${multi_value_args}")

    foreach(required_arg
            TOKEN_FILE
            PALETTE_FILE
            SCRIPT
            OUTPUT_TARGET_VAR)
        if(NOT VFT_${required_arg})
            message(FATAL_ERROR
                "[validate_font_tokens] ${required_arg} is required")
        endif()
    endforeach()
    if(NOT VFT_THEME_FILES)
        message(FATAL_ERROR
            "[validate_font_tokens] THEME_FILES is required")
    endif()
    if(VFT_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[validate_font_tokens] Arguments missing values: "
            "${VFT_KEYWORDS_MISSING_VALUES}")
    endif()
    if(VFT_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[validate_font_tokens] Unknown arguments: "
            "${VFT_UNPARSED_ARGUMENTS}")
    endif()
    _tt_token_validation_validate_output_variable(
        FUNCTION_NAME validate_font_tokens
        VARIABLE_NAME "${VFT_OUTPUT_TARGET_VAR}"
    )

    _tt_token_validation_normalize_path(
        PATH "${VFT_PALETTE_FILE}"
        BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE palette_file
    )

    _tt_register_token_validation(
        KIND font
        SCRIPT "${VFT_SCRIPT}"
        TOKEN_FILE "${VFT_TOKEN_FILE}"
        THEME_FILES ${VFT_THEME_FILES}
        ADDITIONAL_DEPENDS "${palette_file}"
        EXTRA_ARGUMENTS --palette "${palette_file}"
        COMMENT "Validating font token and palette consistency"
        OUTPUT_TARGET_VAR validation_target
    )

    set("${VFT_OUTPUT_TARGET_VAR}" "${validation_target}" PARENT_SCOPE)
endfunction()
