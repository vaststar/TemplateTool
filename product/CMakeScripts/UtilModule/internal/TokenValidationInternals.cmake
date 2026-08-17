include_guard()

function(_tt_token_validation_normalize_path)
    set(one_value_args PATH BASE_DIRECTORY OUTPUT_VARIABLE)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "${one_value_args}" "")

    foreach(required_arg PATH BASE_DIRECTORY OUTPUT_VARIABLE)
        if(NOT ARG_${required_arg})
            message(FATAL_ERROR
                "[_tt_token_validation_normalize_path] ${required_arg} is required")
        endif()
    endforeach()
    if(ARG_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[_tt_token_validation_normalize_path] Arguments missing values: "
            "${ARG_KEYWORDS_MISSING_VALUES}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[_tt_token_validation_normalize_path] Unknown arguments: "
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

function(_tt_token_validation_validate_output_variable)
    set(one_value_args FUNCTION_NAME VARIABLE_NAME)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "${one_value_args}" "")

    foreach(required_arg FUNCTION_NAME VARIABLE_NAME)
        if(NOT ARG_${required_arg})
            message(FATAL_ERROR
                "[_tt_token_validation_validate_output_variable] "
                "${required_arg} is required")
        endif()
    endforeach()
    if(ARG_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[_tt_token_validation_validate_output_variable] "
            "Arguments missing values: ${ARG_KEYWORDS_MISSING_VALUES}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[_tt_token_validation_validate_output_variable] "
            "Unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT "${ARG_VARIABLE_NAME}" MATCHES "^[A-Za-z_][A-Za-z0-9_]*$")
        message(FATAL_ERROR
            "[${ARG_FUNCTION_NAME}] OUTPUT_TARGET_VAR must be a valid "
            "variable name, got '${ARG_VARIABLE_NAME}'")
    endif()
endfunction()

function(_tt_register_token_validation)
    set(one_value_args
        KIND
        SCRIPT
        TOKEN_FILE
        OUTPUT_TARGET_VAR
        COMMENT
    )
    set(multi_value_args
        THEME_FILES
        ADDITIONAL_DEPENDS
        EXTRA_ARGUMENTS
    )
    cmake_parse_arguments(
        PARSE_ARGV 0 ARG "" "${one_value_args}" "${multi_value_args}")

    foreach(required_arg KIND SCRIPT TOKEN_FILE OUTPUT_TARGET_VAR COMMENT)
        if(NOT ARG_${required_arg})
            message(FATAL_ERROR
                "[_tt_register_token_validation] ${required_arg} is required")
        endif()
    endforeach()
    if(NOT ARG_THEME_FILES)
        message(FATAL_ERROR
            "[_tt_register_token_validation] THEME_FILES is required")
    endif()
    if(ARG_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[_tt_register_token_validation] Arguments missing values: "
            "${ARG_KEYWORDS_MISSING_VALUES}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[_tt_register_token_validation] Unknown arguments: "
            "${ARG_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT "${ARG_OUTPUT_TARGET_VAR}" MATCHES "^[A-Za-z_][A-Za-z0-9_]*$")
        message(FATAL_ERROR
            "[_tt_register_token_validation] OUTPUT_TARGET_VAR must be a "
            "valid variable name, got '${ARG_OUTPUT_TARGET_VAR}'")
    endif()

    _tt_token_validation_normalize_path(
        PATH "${ARG_SCRIPT}"
        BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE script_file
    )
    _tt_token_validation_normalize_path(
        PATH "${ARG_TOKEN_FILE}"
        BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE token_file
    )

    set(theme_files)
    foreach(theme_file IN LISTS ARG_THEME_FILES)
        _tt_token_validation_normalize_path(
            PATH "${theme_file}"
            BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE normalized_theme_file
        )
        list(APPEND theme_files "${normalized_theme_file}")
    endforeach()

    set(additional_depends)
    foreach(dependency_file IN LISTS ARG_ADDITIONAL_DEPENDS)
        _tt_token_validation_normalize_path(
            PATH "${dependency_file}"
            BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE normalized_dependency_file
        )
        list(APPEND additional_depends "${normalized_dependency_file}")
    endforeach()

    foreach(required_file
            "${script_file}"
            "${token_file}"
            ${theme_files}
            ${additional_depends})
        if(NOT EXISTS "${required_file}")
            message(FATAL_ERROR
                "[_tt_register_token_validation] Required file does not exist: "
                "${required_file}")
        endif()
    endforeach()

    string(CONCAT registration_data
        "kind=${ARG_KIND}\n"
        "script=${script_file}\n"
        "token=${token_file}\n"
        "themes=${theme_files}\n"
        "additional_depends=${additional_depends}\n"
        "extra_arguments=${ARG_EXTRA_ARGUMENTS}\n"
    )
    string(SHA256 registration_signature "${registration_data}")
    string(SUBSTRING "${registration_signature}" 0 12 short_signature)
    string(MAKE_C_IDENTIFIER "${ARG_KIND}" safe_kind)
    string(TOLOWER "${safe_kind}" safe_kind)
    set(validation_target
        "validate_${safe_kind}_tokens_${short_signature}")

    if(TARGET "${validation_target}")
        get_target_property(
            registered_signature "${validation_target}"
            TT_TOKEN_VALIDATION_SIGNATURE)
        if(NOT "${registered_signature}" STREQUAL
           "${registration_signature}")
            message(FATAL_ERROR
                "[_tt_register_token_validation] Target-name collision for "
                "'${validation_target}'")
        endif()

        set("${ARG_OUTPUT_TARGET_VAR}" "${validation_target}" PARENT_SCOPE)
        return()
    endif()

    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    set(validation_command
        "${Python3_EXECUTABLE}"
        "${script_file}"
        --token "${token_file}"
        --themes ${theme_files}
        ${ARG_EXTRA_ARGUMENTS}
    )

    add_custom_target("${validation_target}"
        COMMAND ${validation_command}
        DEPENDS
            "${script_file}"
            "${token_file}"
            ${theme_files}
            ${additional_depends}
        COMMENT "${ARG_COMMENT}"
        VERBATIM
        COMMAND_EXPAND_LISTS
    )
    set_target_properties("${validation_target}" PROPERTIES
        FOLDER codegen
        TT_TOKEN_VALIDATION TRUE
        TT_TOKEN_VALIDATION_KIND "${ARG_KIND}"
        TT_TOKEN_VALIDATION_SIGNATURE "${registration_signature}"
    )

    message(STATUS
        "[token_validation] ${ARG_KIND} -> ${validation_target}")
    if(CMAKE_VERBOSE_MAKEFILE)
        message(STATUS "[token_validation]   Token : ${token_file}")
        message(STATUS "[token_validation]   Themes: ${theme_files}")
        message(STATUS "[token_validation]   Script: ${script_file}")
    endif()

    set("${ARG_OUTPUT_TARGET_VAR}" "${validation_target}" PARENT_SCOPE)
endfunction()
