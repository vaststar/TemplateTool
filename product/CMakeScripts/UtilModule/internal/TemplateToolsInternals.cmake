include_guard()

function(_tt_normalize_path)
    set(one_value_args PATH BASE_DIRECTORY OUTPUT_VARIABLE)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "${one_value_args}" "")

    foreach(required_arg PATH BASE_DIRECTORY OUTPUT_VARIABLE)
        if(NOT ARG_${required_arg})
            message(FATAL_ERROR
                "[_tt_normalize_path] ${required_arg} is required")
        endif()
    endforeach()
    if(ARG_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[_tt_normalize_path] Arguments missing values: "
            "${ARG_KEYWORDS_MISSING_VALUES}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[_tt_normalize_path] Unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
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

function(_tt_make_template_target_name)
    set(one_value_args OUTPUT_FILE OUTPUT_VARIABLE)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "${one_value_args}" "")

    if(NOT ARG_OUTPUT_FILE)
        message(FATAL_ERROR
            "[_tt_make_template_target_name] OUTPUT_FILE is required")
    endif()
    if(NOT ARG_OUTPUT_VARIABLE)
        message(FATAL_ERROR
            "[_tt_make_template_target_name] OUTPUT_VARIABLE is required")
    endif()
    if(ARG_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[_tt_make_template_target_name] Arguments missing values: "
            "${ARG_KEYWORDS_MISSING_VALUES}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[_tt_make_template_target_name] Unknown arguments: "
            "${ARG_UNPARSED_ARGUMENTS}")
    endif()

    get_filename_component(output_name "${ARG_OUTPUT_FILE}" NAME)
    string(MAKE_C_IDENTIFIER "${output_name}" safe_output_name)
    string(SHA256 output_hash "${ARG_OUTPUT_FILE}")
    string(SUBSTRING "${output_hash}" 0 12 short_output_hash)

    set("${ARG_OUTPUT_VARIABLE}"
        "generate_${safe_output_name}_${short_output_hash}"
        PARENT_SCOPE
    )
endfunction()

function(_tt_ensure_jinja_runtime)
    set(one_value_args
        REQUIREMENTS_FILE
        OUTPUT_PYTHON_VAR
        OUTPUT_TARGET_VAR
    )
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "${one_value_args}" "")

    foreach(required_arg
            REQUIREMENTS_FILE
            OUTPUT_PYTHON_VAR
            OUTPUT_TARGET_VAR)
        if(NOT ARG_${required_arg})
            message(FATAL_ERROR
                "[_tt_ensure_jinja_runtime] ${required_arg} is required")
        endif()
    endforeach()
    if(ARG_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[_tt_ensure_jinja_runtime] Arguments missing values: "
            "${ARG_KEYWORDS_MISSING_VALUES}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[_tt_ensure_jinja_runtime] Unknown arguments: "
            "${ARG_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT EXISTS "${ARG_REQUIREMENTS_FILE}")
        message(FATAL_ERROR
            "[_tt_ensure_jinja_runtime] Requirements file does not exist: "
            "${ARG_REQUIREMENTS_FILE}")
    endif()

    set(runtime_target jinja_venv)
    if(TARGET "${runtime_target}")
        get_target_property(
            is_jinja_runtime "${runtime_target}" TT_JINJA_RUNTIME)
        get_target_property(
            runtime_python "${runtime_target}" TT_JINJA_PYTHON_EXECUTABLE)
        get_target_property(
            runtime_requirements "${runtime_target}" TT_JINJA_REQUIREMENTS_FILE)

        if(NOT is_jinja_runtime OR NOT runtime_python)
            message(FATAL_ERROR
                "[_tt_ensure_jinja_runtime] Target '${runtime_target}' already "
                "exists but is not a registered TemplateTool Jinja runtime")
        endif()
        if(NOT "${runtime_requirements}" STREQUAL "${ARG_REQUIREMENTS_FILE}")
            message(FATAL_ERROR
                "[_tt_ensure_jinja_runtime] Target '${runtime_target}' was "
                "registered with a different requirements file: "
                "${runtime_requirements}")
        endif()

        set("${ARG_OUTPUT_PYTHON_VAR}" "${runtime_python}" PARENT_SCOPE)
        set("${ARG_OUTPUT_TARGET_VAR}" "${runtime_target}" PARENT_SCOPE)
        return()
    endif()

    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    set(venv_directory "${CMAKE_BINARY_DIR}/jinja_venv")
    set(runtime_marker "${venv_directory}/.venv_ready")
    if(WIN32)
        set(runtime_python "${venv_directory}/Scripts/python.exe")
    else()
        set(runtime_python "${venv_directory}/bin/python")
    endif()

    add_custom_command(
        OUTPUT "${runtime_marker}"
        COMMAND "${CMAKE_COMMAND}" -E rm -rf "${venv_directory}"
        COMMAND "${Python3_EXECUTABLE}" -m venv "${venv_directory}"
        COMMAND "${runtime_python}" -m pip install
            -r "${ARG_REQUIREMENTS_FILE}"
        COMMAND "${CMAKE_COMMAND}" -E touch "${runtime_marker}"
        DEPENDS "${ARG_REQUIREMENTS_FILE}"
        COMMENT "Preparing the TemplateTool Jinja runtime"
        VERBATIM
    )

    add_custom_target("${runtime_target}" DEPENDS "${runtime_marker}")
    set_target_properties("${runtime_target}" PROPERTIES
        FOLDER codegen
        TT_JINJA_RUNTIME TRUE
        TT_JINJA_PYTHON_EXECUTABLE "${runtime_python}"
        TT_JINJA_REQUIREMENTS_FILE "${ARG_REQUIREMENTS_FILE}"
    )

    message(STATUS
        "[generate_from_template] Registered Jinja runtime: ${runtime_target}")
    if(CMAKE_VERBOSE_MAKEFILE)
        message(STATUS
            "[generate_from_template]   System Python: ${Python3_EXECUTABLE}")
        message(STATUS
            "[generate_from_template]   Runtime Python: ${runtime_python}")
        message(STATUS
            "[generate_from_template]   Requirements  : ${ARG_REQUIREMENTS_FILE}")
    endif()

    set("${ARG_OUTPUT_PYTHON_VAR}" "${runtime_python}" PARENT_SCOPE)
    set("${ARG_OUTPUT_TARGET_VAR}" "${runtime_target}" PARENT_SCOPE)
endfunction()
