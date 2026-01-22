include_guard()

# ==========================================
# Script mode: Generate git_info.meta file
# ==========================================
if(CMAKE_SCRIPT_MODE_FILE)
    if(NOT DEFINED OUTPUT_FILE)
        message(FATAL_ERROR "[GenerateGitInfoMeta] OUTPUT_FILE variable not defined")
    endif()

    if(NOT DEFINED SOURCE_DIR)
        message(FATAL_ERROR "[GenerateGitInfoMeta] SOURCE_DIR variable not defined")
    endif()

    find_package(Git QUIET)
    if(GIT_FOUND)
        # Get Git commit depth
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
            WORKING_DIRECTORY "${SOURCE_DIR}"
            OUTPUT_VARIABLE git_depth
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE depth_result
        )

        # Get Git commit hash
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
            WORKING_DIRECTORY "${SOURCE_DIR}"
            OUTPUT_VARIABLE git_commit
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE hash_result
        )

        # Get current Git branch
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY "${SOURCE_DIR}"
            OUTPUT_VARIABLE git_branch
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE branch_result
        )

        # Check if all Git commands succeeded
        if(NOT depth_result EQUAL 0 OR NOT hash_result EQUAL 0 OR NOT branch_result EQUAL 0)
            message(WARNING "[GenerateGitInfoMeta] Git command failed, using fallback values")
            set(git_depth 0)
            set(git_commit "unknown")
            set(git_branch "unknown")
        endif()

        set(NEW_CONTENT "{\n  \"hash\": \"${git_commit}\",\n  \"branch\": \"${git_branch}\",\n  \"depth\": ${git_depth}\n}")
        message(STATUS "[GenerateGitInfoMeta] Git info: branch=${git_branch}, hash=${git_commit}, depth=${git_depth}")

        # Only write if content changed (avoid unnecessary rebuilds)
        if(EXISTS "${OUTPUT_FILE}")
            file(READ "${OUTPUT_FILE}" OLD_CONTENT)
        else()
            set(OLD_CONTENT "")
        endif()

        if(NOT OLD_CONTENT STREQUAL NEW_CONTENT)
            message(STATUS "[GenerateGitInfoMeta] Git information changed, updating ${OUTPUT_FILE}")
            file(WRITE "${OUTPUT_FILE}" "${NEW_CONTENT}")
        else()
            message(STATUS "[GenerateGitInfoMeta] Git information unchanged, skipping write")
        endif()
    else()
        message(WARNING "[GenerateGitInfoMeta] Git not found, cannot determine Git information")
        # Write fallback values
        set(NEW_CONTENT "{\n  \"hash\": \"unknown\",\n  \"branch\": \"unknown\",\n  \"depth\": 0\n}")
        file(WRITE "${OUTPUT_FILE}" "${NEW_CONTENT}")
    endif()
endif()

# ==========================================
# Function: generate_git_info_meta
# ==========================================
function(generate_git_info_meta)
    set(oneValueArgs OUTPUT_FILE)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "" ${ARGN})

    if(NOT ARG_OUTPUT_FILE)
        message(FATAL_ERROR "[GenerateGitInfoMeta] OUTPUT_FILE is required")
    endif()

    message(STATUS "")
    message(STATUS "============================================================")
    message(STATUS "[GenerateGitInfoMeta] Configuring Git info generation")
    message(STATUS "============================================================")
    message(STATUS "  Output File  : ${ARG_OUTPUT_FILE}")
    message(STATUS "  Source Dir   : ${CMAKE_SOURCE_DIR}")

    # Build list of Git dependency files (any change triggers regeneration)
    set(GIT_DEPEND_FILES)
    
    # .git/HEAD changes when switching branches
    if(EXISTS "${CMAKE_SOURCE_DIR}/.git/HEAD")
        list(APPEND GIT_DEPEND_FILES "${CMAKE_SOURCE_DIR}/.git/HEAD")
    endif()
    
    # .git/index changes on commits
    if(EXISTS "${CMAKE_SOURCE_DIR}/.git/index")
        list(APPEND GIT_DEPEND_FILES "${CMAKE_SOURCE_DIR}/.git/index")
    endif()

    list(LENGTH GIT_DEPEND_FILES depend_count)
    message(STATUS "  Git Depends  : ${depend_count} file(s)")
    message(STATUS "------------------------------------------------------------")

    add_custom_command(
        OUTPUT "${ARG_OUTPUT_FILE}"
        COMMAND ${CMAKE_COMMAND}
            -DOUTPUT_FILE=${ARG_OUTPUT_FILE}
            -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/GenerateGitInfoMeta.cmake"
        DEPENDS ${GIT_DEPEND_FILES}
        COMMENT "[GenerateGitInfoMeta] Generating ${ARG_OUTPUT_FILE}"
        VERBATIM
    )
endfunction()