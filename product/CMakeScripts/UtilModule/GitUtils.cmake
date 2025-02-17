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