include_guard()
# 定义一个函数来获取 Git 提交深度、提交哈希和当前分支
function(get_full_git_info depth_variable commit_variable branch_variable)
    # 检查当前目录是否为 Git 仓库
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

        set(${depth_variable} ${git_depth} PARENT_SCOPE)
        set(${commit_variable} ${git_commit} PARENT_SCOPE)
        set(${branch_variable} ${git_branch} PARENT_SCOPE)
    else()
        message(STATUS "Git not found. Cannot determine Git information.")
        set(${depth_variable} "" PARENT_SCOPE)
        set(${commit_variable} "" PARENT_SCOPE)
        set(${branch_variable} "" PARENT_SCOPE)
    endif()
endfunction()

function(GenerateAppVersionFile)
    set(options)  # 没有布尔选项
    set(oneValueArgs VERSION_MAJOR VERSION_MINOR INPUT_VERSION_FILE OUTPUT_FOLDER)
    set(multiValueArgs DEPENDS)
    cmake_parse_arguments(GAVF "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT GAVF_INPUT_VERSION_FILE)
        message(FATAL_ERROR "[GenerateAppVersionFile] Missing required argument: INPUT_VERSION_FILE")
    endif()
    if(NOT GAVF_OUTPUT_FOLDER)
        message(FATAL_ERROR "[GenerateAppVersionFile] Missing required argument: OUTPUT_FOLDER")
    endif()
    if(NOT GAVF_VERSION_MAJOR)
        message(STATUS "[GenerateAppVersionFile] Missing required argument: VERSION_MAJOR")
        set(GAVF_VERSION_MAJOR 1)
    endif()
    if(NOT GAVF_VERSION_MINOR)
        message(STATUS "[GenerateAppVersionFile] Missing required argument: VERSION_MINOR")
        set(GAVF_VERSION_MINOR 0)
    endif()

    # 设置项目版本号
    set(PROJECT_VERSION_MAJOR ${GAVF_VERSION_MAJOR})
    set(PROJECT_VERSION_MINOR ${GAVF_VERSION_MINOR})
    set(PROJECT_VERSION_PATCH 1)

    # 调用函数获取完整的 Git 信息
    get_full_git_info(GIT_COMMIT_DEPTH GIT_COMMIT_HASH GIT_CURRENT_BRANCH)

    # 输出 Git 信息
    if(GIT_COMMIT_DEPTH AND GIT_COMMIT_HASH AND GIT_CURRENT_BRANCH)
        set(PROJECT_VERSION_PATCH ${GIT_COMMIT_DEPTH})
        message(STATUS "Git commit depth: ${GIT_COMMIT_DEPTH}")
        message(STATUS "Git commit hash: ${GIT_COMMIT_HASH}")
        message(STATUS "Git current branch: ${GIT_CURRENT_BRANCH}")
    else()
        message(STATUS "Failed to get full Git information.")
    endif()
    
    # # 读取 version.h 文件
    # if(EXISTS "${CMAKE_BINARY_DIR}/appVersion.h")
    #     # 读取 PROJECT_VERSION_PATCH 的值
    #     file(READ "${CMAKE_BINARY_DIR}/appVersion.h" VERSION_H_CONTENTS)
    #     string(REGEX MATCH "PROJECT_VERSION_PATCH = \"([0-999999]+)\"" _match ${VERSION_H_CONTENTS})
    #     set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_1})
    #     message(STATUS "Read patch: ${PROJECT_VERSION_PATCH}")
    
    #     # 将 PROJECT_VERSION_PATCH 值加 1
    #     math(EXPR PROJECT_VERSION_PATCH "${PROJECT_VERSION_PATCH} + 1")
    #     message(STATUS "更新版本号: ${PROJECT_VERSION}")
    # endif()

    set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

    # 获取编译日期
    # string(TIMESTAMP PROJECT_COMPILE_YEAR "%Y" UTC)
    # string(TIMESTAMP PROJECT_COMPILE_MONTH "%m" UTC)
    # string(TIMESTAMP PROJECT_COMPILE_DAY "%d" UTC)
    string(TIMESTAMP PROJECT_COMPILE_TIME "%Y-%m-%dT%H:%M:%SZ" UTC)
    
    # 生成版本文件
    get_filename_component(VERSION_FILE_NAME ${GAVF_INPUT_VERSION_FILE} NAME)
    string(REGEX REPLACE "\\.in$" "" FILENAME_NO_IN ${VERSION_FILE_NAME})
    configure_file(${GAVF_INPUT_VERSION_FILE} ${GAVF_OUTPUT_FOLDER}/${FILENAME_NO_IN} @ONLY)


    list(LENGTH GAVF_UNPARSED_ARGUMENTS unparsed_count)
    if(NOT unparsed_count EQUAL 1)
        message(FATAL_ERROR "函数调用错误: 需要指定1个输出变量名表示TARGET_NAME")
    endif()
    list(GET GAVF_UNPARSED_ARGUMENTS 0 app_version_target)

    set(MODULE_TARGET_NAME generate_${FILENAME_NO_IN})
    add_library(${MODULE_TARGET_NAME} INTERFACE)
    target_include_directories(${MODULE_TARGET_NAME} INTERFACE ${GAVF_OUTPUT_FOLDER})
    set(${app_version_target} ${MODULE_TARGET_NAME} PARENT_SCOPE)
endfunction()