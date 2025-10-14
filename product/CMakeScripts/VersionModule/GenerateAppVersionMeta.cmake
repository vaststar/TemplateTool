include_guard()
if(CMAKE_SCRIPT_MODE_FILE)
    include("${CMAKE_CURRENT_LIST_DIR}/GitUtils.cmake")
    # 获取 Git 信息
    get_full_git_info(GIT_COMMIT_DEPTH GIT_COMMIT_HASH GIT_COMMIT_BRANCH)
    if(GIT_COMMIT_DEPTH)
        set(VERSION_BUILD ${GIT_COMMIT_DEPTH})
    endif()

    string(TIMESTAMP COMPILE_TIME "%Y-%m-%dT%H:%M:%SZ" UTC)

    if(NOT EXISTS "${INPUT_META_JSON}")
        message(FATAL_ERROR "Version config file not found: ${INPUT_META_JSON}")
    endif()

    file(READ "${INPUT_META_JSON}" JSON_CONTENT)
    # 解析版本信息
    string(JSON VERSION_MAJOR GET "${JSON_CONTENT}" "version" "major")
    string(JSON VERSION_MINOR GET "${JSON_CONTENT}" "version" "minor")
    string(JSON VERSION_PATCH GET "${JSON_CONTENT}" "version" "patch")
    
    # 解析公司信息
    string(JSON COMPANY_NAME GET "${JSON_CONTENT}" "company" "name")
    string(JSON COPYRIGHT GET "${JSON_CONTENT}" "company" "copyright")
    
    # 解析产品信息
    string(JSON PRODUCT_NAME GET "${JSON_CONTENT}" "product" "name")
    string(JSON PRODUCT_DESCRIPTION GET "${JSON_CONTENT}" "product" "description")

    configure_file(${INPUT_TEMPLATE} ${OUTPUT_FILE} @ONLY)
    message(STATUS "[GenerateAppVersionMeta] Generated ${OUTPUT_FILE}")
endif()

function(generate_app_version_meta)
    set(oneValueArgs VERSION_MAJOR VERSION_MINOR VERSION_PATCH INPUT_META_JSON INPUT_VERSION_TEMPLATE OUTPUT_FILE)
    cmake_parse_arguments(GAVM "" "${oneValueArgs}" "" ${ARGN})

    if(NOT GAVM_OUTPUT_FILE)
        message(FATAL_ERROR "[GenerateAppVersionMeta] OUTPUT_FILE is required")
    endif()

    set(TARGET_NAME generate_app_version_meta)
    add_custom_target(${TARGET_NAME} ALL
        COMMAND ${CMAKE_COMMAND}
            -DINPUT_META_JSON=${GAVM_INPUT_META_JSON}
            -DINPUT_TEMPLATE=${GAVM_INPUT_VERSION_TEMPLATE}
            -DOUTPUT_FILE=${GAVM_OUTPUT_FILE}
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/GenerateAppVersionMeta.cmake"
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