include_guard()
if(CMAKE_SCRIPT_MODE_FILE)
    file(READ "${INPUT_JSON_FILE}" json_content)
     # 解析 JSON 内容
    string(JSON VERSION_MAJOR GET "${json_content}" "VERSION" "VERSION_MAJOR")
    string(JSON VERSION_MINOR GET "${json_content}" "VERSION" "VERSION_MINOR")
    string(JSON VERSION_PATCH GET "${json_content}" "VERSION" "VERSION_PATCH")
    string(JSON VERSION_BUILD GET "${json_content}" "VERSION" "VERSION_BUILD")
    string(JSON GIT_COMMIT_HASH GET "${json_content}" "COMPILATION" "GIT_COMMIT_HASH")
    string(JSON GIT_COMMIT_BRANCH GET "${json_content}" "COMPILATION" "GIT_COMMIT_BRANCH")
    string(JSON COMPILE_TIME GET "${json_content}" "COMPILATION" "COMPILE_TIME")

    # 获取传入变量
    configure_file(${INPUT_TEMPLATE_FILE} ${OUTPUT_H} @ONLY)
    message(STATUS "[GenerateAppVersionHeader] Generated ${OUTPUT_H}")
endif()

function(generate_app_version_header)
    set(options)  # 没有布尔选项
    set(oneValueArgs INPUT_JSON_FILE INPUT_JSON_TARGET INPUT_VERSION_TEMPLATE OUTPUT_FILE)
    set(multiValueArgs DEPENDS)
    cmake_parse_arguments(GAVF "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT GAVF_INPUT_VERSION_TEMPLATE)
        message(FATAL_ERROR "[GenerateAppVersionHeader] Missing required argument: INPUT_VERSION_TEMPLATE")
    endif()
    if(NOT GAVF_INPUT_JSON_TARGET)
        message(FATAL_ERROR "[GenerateAppVersionHeader] Missing required argument: INPUT_JSON_TARGET")
    endif()
    if(NOT GAVF_OUTPUT_FILE)
        message(FATAL_ERROR "[GenerateAppVersionHeader] Missing required argument: OUTPUT_FILE")
    endif()
    if(NOT GAVF_INPUT_JSON_FILE)
        message(FATAL_ERROR "[GenerateAppVersionHeader] Missing required argument: INPUT_JSON_FILE")
    endif()
    
    # 生成版本文件
    set(CUSTOM_TARGET_NAME "generate_app_version_header")
    add_custom_command(
        OUTPUT ${GAVF_OUTPUT_FILE}
        COMMAND ${CMAKE_COMMAND} -DINPUT_JSON_FILE=${GAVF_INPUT_JSON_FILE}
                                -DINPUT_TEMPLATE_FILE=${GAVF_INPUT_VERSION_TEMPLATE}
                                -DOUTPUT_H=${GAVF_OUTPUT_FILE}
                                -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/GenerateAppVersionHeader.cmake"
        COMMENT "Generating ${GAVF_OUTPUT_FILE} from ${GAVF_INPUT_JSON_FILE} using ${GAVF_INPUT_VERSION_TEMPLATE}"
        DEPENDS ${GAVF_INPUT_JSON_FILE}
    )
    add_custom_target(${CUSTOM_TARGET_NAME} ALL DEPENDS ${GAVF_OUTPUT_FILE})
    set_target_properties(${CUSTOM_TARGET_NAME} PROPERTIES FOLDER codegen)
    add_dependencies(${CUSTOM_TARGET_NAME} ${GAVF_INPUT_JSON_TARGET})
    
    list(LENGTH GAVF_UNPARSED_ARGUMENTS unparsed_count)
    if(NOT unparsed_count EQUAL 1)
        message(FATAL_ERROR "函数调用错误: 需要指定1个输出变量名表示TARGET_NAME")
    endif()
    list(GET GAVF_UNPARSED_ARGUMENTS 0 app_version_target)
    set(${app_version_target} ${CUSTOM_TARGET_NAME} PARENT_SCOPE)
endfunction()