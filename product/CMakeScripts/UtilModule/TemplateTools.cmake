include_guard()

include("${CMAKE_CURRENT_LIST_DIR}/internal/TemplateToolsInternals.cmake")

# Generate a file from a Jinja template and structured input.
function(generate_from_template)
    set(one_value_args
        TEMPLATE_FILE
        INPUT_FILE
        OUTPUT_FILE
        OUTPUT_TARGET_VAR
    )
    set(multi_value_args DEPENDS EXTRA_PARAMS)
    cmake_parse_arguments(
        PARSE_ARGV 0 GFT "" "${one_value_args}" "${multi_value_args}")

    foreach(required_arg
            TEMPLATE_FILE
            INPUT_FILE
            OUTPUT_FILE
            OUTPUT_TARGET_VAR)
        if(NOT GFT_${required_arg})
            message(FATAL_ERROR
                "[generate_from_template] ${required_arg} is required")
        endif()
    endforeach()
    if(GFT_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[generate_from_template] Arguments missing values: "
            "${GFT_KEYWORDS_MISSING_VALUES}")
    endif()
    if(GFT_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[generate_from_template] Unknown arguments: "
            "${GFT_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT "${GFT_OUTPUT_TARGET_VAR}" MATCHES "^[A-Za-z_][A-Za-z0-9_]*$")
        message(FATAL_ERROR
            "[generate_from_template] OUTPUT_TARGET_VAR must be a valid "
            "variable name, got '${GFT_OUTPUT_TARGET_VAR}'")
    endif()

    _tt_normalize_path(
        PATH "${GFT_TEMPLATE_FILE}"
        BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE template_file
    )
    _tt_normalize_path(
        PATH "${GFT_INPUT_FILE}"
        BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE input_file
    )
    _tt_normalize_path(
        PATH "${GFT_OUTPUT_FILE}"
        BASE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        OUTPUT_VARIABLE output_file
    )

    set(renderer_directory
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../../codegen/template_render")
    cmake_path(NORMAL_PATH renderer_directory)
    set(renderer_script "${renderer_directory}/render_template.py")
    set(requirements_file "${renderer_directory}/requirements.txt")

    foreach(required_file
            "${template_file}"
            "${input_file}"
            "${renderer_script}"
            "${requirements_file}")
        if(NOT EXISTS "${required_file}")
            message(FATAL_ERROR
                "[generate_from_template] Required file does not exist: "
                "${required_file}")
        endif()
    endforeach()

    _tt_make_template_target_name(
        OUTPUT_FILE "${output_file}"
        OUTPUT_VARIABLE generated_target
    )

    string(CONCAT registration_data
        "template=${template_file}\n"
        "input=${input_file}\n"
        "output=${output_file}\n"
        "depends=${GFT_DEPENDS}\n"
        "extra_params=${GFT_EXTRA_PARAMS}\n"
    )
    string(SHA256 registration_signature "${registration_data}")

    if(TARGET "${generated_target}")
        get_target_property(
            registered_output "${generated_target}" TT_GENERATED_OUTPUT)
        get_target_property(
            registered_signature "${generated_target}" TT_GENERATOR_SIGNATURE)

        if(NOT "${registered_output}" STREQUAL "${output_file}")
            message(FATAL_ERROR
                "[generate_from_template] Target-name collision for "
                "'${generated_target}': '${registered_output}' and "
                "'${output_file}'")
        endif()
        if(NOT "${registered_signature}" STREQUAL
           "${registration_signature}")
            message(FATAL_ERROR
                "[generate_from_template] Output '${output_file}' is already "
                "registered with a different template-generation configuration")
        endif()

        set("${GFT_OUTPUT_TARGET_VAR}" "${generated_target}" PARENT_SCOPE)
        return()
    endif()

    _tt_ensure_jinja_runtime(
        REQUIREMENTS_FILE "${requirements_file}"
        OUTPUT_PYTHON_VAR runtime_python
        OUTPUT_TARGET_VAR runtime_target
    )

    get_filename_component(output_directory "${output_file}" DIRECTORY)
    file(MAKE_DIRECTORY "${output_directory}")

    set(render_command
        "${runtime_python}"
        "${renderer_script}"
        --template "${template_file}"
        --input "${input_file}"
        --output "${output_file}"
    )
    foreach(extra_param IN LISTS GFT_EXTRA_PARAMS)
        list(APPEND render_command --param "${extra_param}")
    endforeach()

    add_custom_command(
        OUTPUT "${output_file}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${output_directory}"
        COMMAND ${render_command}
        DEPENDS
            "${runtime_target}"
            "${template_file}"
            "${input_file}"
            "${renderer_script}"
            "${requirements_file}"
            ${GFT_DEPENDS}
        COMMENT "Generating ${output_file}"
        VERBATIM
    )

    add_custom_target("${generated_target}" ALL DEPENDS "${output_file}")
    add_dependencies("${generated_target}" "${runtime_target}")
    set_target_properties("${generated_target}" PROPERTIES
        FOLDER codegen
        TT_GENERATED_OUTPUT "${output_file}"
        TT_GENERATOR_SIGNATURE "${registration_signature}"
    )
    set_source_files_properties("${output_file}" PROPERTIES GENERATED TRUE)

    message(STATUS
        "[generate_from_template] ${generated_target} -> ${output_file}")
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS
            "[generate_from_template]   Template: ${template_file}")
        message(STATUS
            "[generate_from_template]   Input   : ${input_file}")
        message(STATUS
            "[generate_from_template]   Depends : ${GFT_DEPENDS}")
        message(STATUS
            "[generate_from_template]   Params  : ${GFT_EXTRA_PARAMS}")
    endif()

    set("${GFT_OUTPUT_TARGET_VAR}" "${generated_target}" PARENT_SCOPE)
endfunction()
