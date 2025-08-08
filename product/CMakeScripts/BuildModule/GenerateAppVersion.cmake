include_guard()
include(GitUtils)

function(GenerateAppVersionFile)
    # 设置项目版本号
    set(PROJECT_VERSION_MAJOR 1)
    set(PROJECT_VERSION_MINOR 0)
    set(PROJECT_VERSION_PATCH 1)

    # 调用函数获取完整的 Git 信息
    get_full_git_info(GIT_COMMIT_DEPTH GIT_COMMIT_HASH GIT_CURRENT_BRANCH)

    # 输出 Git 信息
    if(GIT_COMMIT_DEPTH AND GIT_COMMIT_HASH AND GIT_CURRENT_BRANCH)
        message(STATUS "Git commit depth: ${GIT_COMMIT_DEPTH}")
        message(STATUS "Git commit hash: ${GIT_COMMIT_HASH}")
        message(STATUS "Git current branch: ${GIT_CURRENT_BRANCH}")
    else()
        message(STATUS "Failed to get full Git information.")
    endif()
    
    # 读取 version.h 文件
    if(EXISTS "${CMAKE_BINARY_DIR}/appVersion.h")
        # 读取 PROJECT_VERSION_PATCH 的值
        file(READ "${CMAKE_BINARY_DIR}/appVersion.h" VERSION_H_CONTENTS)
        string(REGEX MATCH "PROJECT_VERSION_PATCH = \"([0-999999]+)\"" _match ${VERSION_H_CONTENTS})
        set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_1})
        message(STATUS "Read patch: ${PROJECT_VERSION_PATCH}")
    
        # 将 PROJECT_VERSION_PATCH 值加 1
        math(EXPR PROJECT_VERSION_PATCH "${PROJECT_VERSION_PATCH} + 1")
        message(STATUS "更新版本号: ${PROJECT_VERSION}")
    endif()

    set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

    # 获取编译日期
    # string(TIMESTAMP PROJECT_COMPILE_YEAR "%Y" UTC)
    # string(TIMESTAMP PROJECT_COMPILE_MONTH "%m" UTC)
    # string(TIMESTAMP PROJECT_COMPILE_DAY "%d" UTC)
    string(TIMESTAMP PROJECT_COMPILE_TIME "%Y-%m-%dT%H:%M:%SZ" UTC)
    
    # 生成版本文件
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/appVersion.h.in ${CMAKE_BINARY_DIR}/appVersion.h @ONLY)
endfunction()
