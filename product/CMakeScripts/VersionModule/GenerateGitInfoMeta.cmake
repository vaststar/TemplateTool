include_guard()

if(CMAKE_SCRIPT_MODE_FILE)
    foreach(required_variable OUTPUT_FILE SOURCE_DIR)
        if(NOT DEFINED ${required_variable}
           OR "${${required_variable}}" STREQUAL "")
            message(FATAL_ERROR
                "[GenerateGitInfoMeta] ${required_variable} is required")
        endif()
    endforeach()

    find_package(Git QUIET)
    set(git_commands_succeeded FALSE)

    if(GIT_FOUND)
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-list --count HEAD
            WORKING_DIRECTORY "${SOURCE_DIR}"
            OUTPUT_VARIABLE git_depth
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE depth_result
            ERROR_QUIET
        )
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
            WORKING_DIRECTORY "${SOURCE_DIR}"
            OUTPUT_VARIABLE git_commit
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE hash_result
            ERROR_QUIET
        )
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY "${SOURCE_DIR}"
            OUTPUT_VARIABLE git_branch
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE branch_result
            ERROR_QUIET
        )

        if(depth_result EQUAL 0
           AND hash_result EQUAL 0
           AND branch_result EQUAL 0)
            set(git_commands_succeeded TRUE)
        endif()
    endif()

    if(git_commands_succeeded)
        string(CONCAT new_content
            "{\n"
            "  \"hash\": \"${git_commit}\",\n"
            "  \"branch\": \"${git_branch}\",\n"
            "  \"depth\": ${git_depth}\n"
            "}\n"
        )
    else()
        message(WARNING
            "[GenerateGitInfoMeta] Git information is unavailable; "
            "using fallback values")
        string(CONCAT new_content
            "{\n"
            "  \"hash\": \"unknown\",\n"
            "  \"branch\": \"unknown\",\n"
            "  \"depth\": 0\n"
            "}\n"
        )
    endif()

    if(EXISTS "${OUTPUT_FILE}")
        file(READ "${OUTPUT_FILE}" old_content)
    else()
        set(old_content "")
    endif()

    if(NOT "${old_content}" STREQUAL "${new_content}")
        get_filename_component(output_directory "${OUTPUT_FILE}" DIRECTORY)
        file(MAKE_DIRECTORY "${output_directory}")
        set(temporary_output "${OUTPUT_FILE}.tmp")
        file(WRITE "${temporary_output}" "${new_content}")
        file(RENAME "${temporary_output}" "${OUTPUT_FILE}")
        message(STATUS "[GenerateGitInfoMeta] Updated ${OUTPUT_FILE}")
    endif()
else()
    include(
        "${CMAKE_CURRENT_LIST_DIR}/internal/VersionGenerationInternals.cmake")

    function(generate_git_info_meta)
        set(one_value_args OUTPUT_FILE OUTPUT_TARGET_VAR)
        cmake_parse_arguments(
            PARSE_ARGV 0 ARG "" "${one_value_args}" "")

        foreach(required_arg OUTPUT_FILE OUTPUT_TARGET_VAR)
            if(NOT ARG_${required_arg})
                message(FATAL_ERROR
                    "[generate_git_info_meta] ${required_arg} is required")
            endif()
        endforeach()
        if(ARG_KEYWORDS_MISSING_VALUES)
            message(FATAL_ERROR
                "[generate_git_info_meta] Arguments missing values: "
                "${ARG_KEYWORDS_MISSING_VALUES}")
        endif()
        if(ARG_UNPARSED_ARGUMENTS)
            message(FATAL_ERROR
                "[generate_git_info_meta] Unknown arguments: "
                "${ARG_UNPARSED_ARGUMENTS}")
        endif()

        _tt_version_validate_output_variable(
            FUNCTION_NAME generate_git_info_meta
            VARIABLE_NAME "${ARG_OUTPUT_TARGET_VAR}"
        )
        _tt_version_normalize_path(
            PATH "${ARG_OUTPUT_FILE}"
            BASE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            OUTPUT_VARIABLE output_file
        )
        _tt_version_make_target_name(
            PREFIX refresh_git_info
            OUTPUT_FILE "${output_file}"
            OUTPUT_VARIABLE git_info_target
        )

        set(generator_script "${CMAKE_CURRENT_FUNCTION_LIST_FILE}")
        string(CONCAT registration_data
            "output=${output_file}\n"
            "source=${CMAKE_SOURCE_DIR}\n"
            "script=${generator_script}\n"
        )
        string(SHA256 registration_signature "${registration_data}")

        _tt_version_check_registration(
            TARGET "${git_info_target}"
            OUTPUT_FILE "${output_file}"
            SIGNATURE "${registration_signature}"
            OUTPUT_REUSED_VAR reused
        )
        if(reused)
            set("${ARG_OUTPUT_TARGET_VAR}"
                "${git_info_target}"
                PARENT_SCOPE
            )
            return()
        endif()

        get_filename_component(output_directory "${output_file}" DIRECTORY)
        add_custom_target("${git_info_target}"
            COMMAND
                "${CMAKE_COMMAND}" -E make_directory "${output_directory}"
            COMMAND
                "${CMAKE_COMMAND}"
                "-DOUTPUT_FILE=${output_file}"
                "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}"
                -P "${generator_script}"
            BYPRODUCTS "${output_file}"
            COMMENT "Refreshing Git metadata"
            VERBATIM
        )

        _tt_version_record_registration(
            TARGET "${git_info_target}"
            OUTPUT_FILE "${output_file}"
            SIGNATURE "${registration_signature}"
        )
        set_source_files_properties(
            "${output_file}" PROPERTIES GENERATED TRUE)

        message(STATUS
            "[generate_git_info_meta] ${git_info_target} -> ${output_file}")
        set("${ARG_OUTPUT_TARGET_VAR}"
            "${git_info_target}"
            PARENT_SCOPE
        )
    endfunction()
endif()
