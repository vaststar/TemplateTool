
include(GitUtils)
# include(GenerateAppVersionMeta)
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

    set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

    # 获取编译日期
    # string(TIMESTAMP PROJECT_COMPILE_TIME "%Y-%m-%dT%H:%M:%SZ" UTC)
    
    # 生成版本文件
    get_filename_component(VERSION_FILE_NAME ${GAVF_INPUT_VERSION_FILE} NAME)
    string(REGEX REPLACE "\\.in$" "" FILENAME_NO_IN ${VERSION_FILE_NAME})


    # configure_file(${GAVF_INPUT_VERSION_FILE} ${GAVF_OUTPUT_FOLDER}/${FILENAME_NO_IN} @ONLY)

    set(VERSION_OUTPUT_HEADER_FILE "${GAVF_OUTPUT_FOLDER}/${FILENAME_NO_IN}")
    set(CUSTOM_TARGET_ALWAYS_NAME "generate_${FILENAME_NO_IN}_always")
    set(VERSION_OUTPUT_JSON_FILE "${GAVF_OUTPUT_FOLDER}/version_meta.json")
    # GenerateAppVersionMeta(
    #     VERSION_MAJOR ${PROJECT_VERSION_MAJOR}
    #     VERSION_MINOR ${PROJECT_VERSION_MINOR}
    #     OUTPUT_FILE ${VERSION_OUTPUT_JSON_FILE}
    # )
    add_custom_target(${CUSTOM_TARGET_ALWAYS_NAME} ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${GAVF_OUTPUT_FOLDER}"

        COMMAND ${CMAKE_COMMAND} -DMAJOR=${PROJECT_VERSION_MAJOR}
                                -DMINOR=${PROJECT_VERSION_MINOR}
                                -DPATCH=${PROJECT_VERSION_PATCH}
                                -DGIT_HASH=${GIT_COMMIT_HASH}
                                -DGIT_BRANCH=${GIT_CURRENT_BRANCH}
                                -DBUILD_TIME=${PROJECT_COMPILE_TIME}
                                -DINPUT_TEMPLATE_FILE=${GAVF_INPUT_VERSION_FILE}
                                -DOUTPUT_H=${VERSION_OUTPUT_HEADER_FILE}
                                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateVersionFiles.cmake"
    )
    set_target_properties(${CUSTOM_TARGET_ALWAYS_NAME} PROPERTIES FOLDER codegen)
    # add_dependencies(${CUSTOM_TARGET_ALWAYS_NAME} generate_app_version_meta_always)

    
    list(LENGTH GAVF_UNPARSED_ARGUMENTS unparsed_count)
    if(NOT unparsed_count EQUAL 1)
        message(FATAL_ERROR "函数调用错误: 需要指定1个输出变量名表示TARGET_NAME")
    endif()
    list(GET GAVF_UNPARSED_ARGUMENTS 0 app_version_target)

    set(MODULE_TARGET_NAME generate_${FILENAME_NO_IN})
    add_library(${MODULE_TARGET_NAME} INTERFACE)
    target_include_directories(${MODULE_TARGET_NAME} INTERFACE ${GAVF_OUTPUT_FOLDER})
    add_dependencies(${MODULE_TARGET_NAME} ${CUSTOM_TARGET_ALWAYS_NAME})
    set(${app_version_target} ${MODULE_TARGET_NAME} PARENT_SCOPE)
endfunction()