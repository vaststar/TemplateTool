if(CMAKE_SCRIPT_MODE_FILE)
    include("${CMAKE_CURRENT_LIST_DIR}/GitUtils.cmake")
    # 获取 Git 信息
    get_full_git_info(GIT_COMMIT_DEPTH GIT_COMMIT_HASH GIT_CURRENT_BRANCH)
    if(GIT_COMMIT_DEPTH)
        set(VERSION_PATCH ${GIT_COMMIT_DEPTH})
    endif()

    string(TIMESTAMP BUILD_TIME "%Y-%m-%dT%H:%M:%SZ" UTC)
    file(WRITE "${OUTPUT_FILE}" "{\n")
    file(APPEND "${OUTPUT_FILE}" "  \"PROJECT_VERSION_MAJOR\": \"${VERSION_MAJOR}\",\n")
    file(APPEND "${OUTPUT_FILE}" "  \"PROJECT_VERSION_MINOR\": \"${VERSION_MINOR}\",\n")
    file(APPEND "${OUTPUT_FILE}" "  \"PROJECT_VERSION_PATCH\": \"${VERSION_PATCH}\",\n")
    file(APPEND "${OUTPUT_FILE}" "  \"GIT_COMMIT_HASH\": \"${GIT_COMMIT_HASH}\",\n")
    file(APPEND "${OUTPUT_FILE}" "  \"GIT_CURRENT_BRANCH\": \"${GIT_CURRENT_BRANCH}\",\n")
    file(APPEND "${OUTPUT_FILE}" "  \"PROJECT_COMPILE_TIME\": \"${BUILD_TIME}\"\n")
    file(APPEND "${OUTPUT_FILE}" "}\n")
    message(STATUS "[GenerateAppVersionMeta] Generated ${OUTPUT_FILE}")
endif()

function(generate_app_version_meta)
    set(oneValueArgs VERSION_MAJOR VERSION_MINOR OUTPUT_FILE)
    cmake_parse_arguments(GAVM "" "${oneValueArgs}" "" ${ARGN})

    if(NOT GAVM_OUTPUT_FILE)
        message(FATAL_ERROR "[GenerateAppVersionMeta] OUTPUT_FILE is required")
    endif()

    set(TARGET_NAME generate_app_version_meta)
    add_custom_target(${TARGET_NAME} ALL
        COMMAND ${CMAKE_COMMAND}
            -DVERSION_MAJOR=${GAVM_VERSION_MAJOR}
            -DVERSION_MINOR=${GAVM_VERSION_MINOR}
            -DOUTPUT_FILE=${GAVM_OUTPUT_FILE}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateAppVersionMeta.cmake"
        COMMENT "Generating version meta JSON: ${GAVM_OUTPUT_FILE}"
        VERBATIM
    )
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER codegen)
    
    list(LENGTH GAVM_UNPARSED_ARGUMENTS unparsed_count)
    if(NOT unparsed_count EQUAL 1)
        message(FATAL_ERROR "函数调用错误: 需要指定1个输出变量名表示TARGET_NAME")
    endif()
    list(GET GAVM_UNPARSED_ARGUMENTS 0 app_version_target)
    set(${app_version_target} ${TARGET_NAME} PARENT_SCOPE)
endfunction()