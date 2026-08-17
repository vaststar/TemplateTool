include_guard()

include("${CMAKE_CURRENT_LIST_DIR}/internal/TokenValidationInternals.cmake")

function(validate_asset_tokens)
    set(one_value_args
        TOKEN_FILE
        ASSET_ROOT
        SCRIPT
        OUTPUT_TARGET_VAR
    )
    set(multi_value_args THEME_FILES)
    cmake_parse_arguments(
        PARSE_ARGV 0 VAT "" "${one_value_args}" "${multi_value_args}")

    foreach(required_arg TOKEN_FILE ASSET_ROOT SCRIPT OUTPUT_TARGET_VAR)
        if(NOT VAT_${required_arg})
            message(FATAL_ERROR
                "[validate_asset_tokens] ${required_arg} is required")
        endif()
    endforeach()
    if(NOT VAT_THEME_FILES)
        message(FATAL_ERROR
            "[validate_asset_tokens] THEME_FILES is required")
    endif()
    if(VAT_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[validate_asset_tokens] Arguments missing values: "
            "${VAT_KEYWORDS_MISSING_VALUES}")
    endif()
    if(VAT_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[validate_asset_tokens] Unknown arguments: "
            "${VAT_UNPARSED_ARGUMENTS}")
    endif()
    _tt_token_validation_validate_output_variable(
        FUNCTION_NAME validate_asset_tokens
        VARIABLE_NAME "${VAT_OUTPUT_TARGET_VAR}"
    )

    _tt_token_validation_normalize_path(
        PATH "${VAT_ASSET_ROOT}"
        BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE asset_root
    )
    if(NOT IS_DIRECTORY "${asset_root}")
        message(FATAL_ERROR
            "[validate_asset_tokens] ASSET_ROOT is not a directory: "
            "${asset_root}")
    endif()

    _tt_register_token_validation(
        KIND asset
        SCRIPT "${VAT_SCRIPT}"
        TOKEN_FILE "${VAT_TOKEN_FILE}"
        THEME_FILES ${VAT_THEME_FILES}
        EXTRA_ARGUMENTS --asset-root "${asset_root}"
        COMMENT "Validating asset tokens and referenced files"
        OUTPUT_TARGET_VAR validation_target
    )

    set("${VAT_OUTPUT_TARGET_VAR}" "${validation_target}" PARENT_SCOPE)
endfunction()
