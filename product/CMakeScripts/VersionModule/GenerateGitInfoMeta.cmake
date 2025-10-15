include_guard()
if(CMAKE_SCRIPT_MODE_FILE)
    if(NOT DEFINED OUTPUT_FILE)
        message(FATAL_ERROR "OUTPUT variable not defined")
    endif()

    find_package(Git)
    if(GIT_FOUND)
        # 获取 Git 提交深度
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE git_depth
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        # 获取 Git 提交哈希
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE git_commit
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        # 获取当前 Git 分支
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE git_branch
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        set(NEW_CONTENT "{\n  \"hash\": \"${git_commit}\",\n  \"branch\": \"${git_branch}\",\n  \"depth\": ${git_depth}\n}")
        message(STATUS "Git information: ${NEW_CONTENT}")

        if(EXISTS "${OUTPUT_FILE}")
            file(READ "${OUTPUT_FILE}" OLD_CONTENT)
        else()
            set(OLD_CONTENT "")
        endif()

        if(NOT OLD_CONTENT STREQUAL NEW_CONTENT)
            message(STATUS "Git information has changed.")
            file(WRITE "${OUTPUT_FILE}" "${NEW_CONTENT}")
        else()
            message(STATUS "Git information is up to date. No changes made.")
        endif()
    else()
        message(STATUS "Git not found. Cannot determine Git information.")
    endif()
endif()

function(generate_git_info_meta)
    set(oneValueArgs OUTPUT_FILE)
    cmake_parse_arguments(GARG "" "${oneValueArgs}" "" ${ARGN})

    if(NOT GARG_OUTPUT_FILE)
        message(FATAL_ERROR "[GenerateGitInfoMeta] OUTPUT_FILE is required")
    endif()
    message(STATUS "[GenerateGitInfoMeta] ${GARG_OUTPUT_FILE}")
    add_custom_command(
        OUTPUT "${GARG_OUTPUT_FILE}"
        COMMAND ${CMAKE_COMMAND}
            -DOUTPUT_FILE=${GARG_OUTPUT_FILE}
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/GenerateGitInfoMeta.cmake"
        DEPENDS "${CMAKE_SOURCE_DIR}/.git/refs/heads/main" ${GARG_OUTPUT_FILE}
        COMMENT "Generating ${GARG_OUTPUT_FILE}"
        VERBATIM
    )
endfunction()