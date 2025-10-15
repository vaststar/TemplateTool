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

    string(JSON COMPANY_NAME GET "${json_content}" "COMPANY" "NAME")
    string(JSON COPYRIGHT GET "${json_content}" "COMPANY" "COPYRIGHT")
    string(JSON PRODUCT_NAME GET "${json_content}" "PRODUCT" "NAME")
    string(JSON PRODUCT_DESCRIPTION GET "${json_content}" "PRODUCT" "DESCRIPTION")

    # 计算版本号
    set(FILE_VERSION "${VERSION_MAJOR},${VERSION_MINOR},${VERSION_PATCH},${VERSION_BUILD}")
    set(PRODUCT_VERSION "${VERSION_MAJOR},${VERSION_MINOR},${VERSION_PATCH},${VERSION_BUILD}")
    set(FILE_VERSION_STR "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_BUILD}")
    set(PRODUCT_VERSION_STR "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_BUILD}")

    # 获取传入变量
    configure_file(${INPUT_TEMPLATE_FILE} ${OUTPUT_H} @ONLY)
    message(STATUS "[GenerateAppRCFiles] Generated ${OUTPUT_H}")
endif()

function(generate_app_rc_files)
    set(options)  # 没有布尔选项
    set(oneValueArgs INPUT_JSON_FILE INPUT_JSON_TARGET INPUT_VERSION_TEMPLATE OUTPUT_FILE INTERNAL_NAME FILE_DESCRIPTION ORIGINAL_FILENAME)
    set(multiValueArgs DEPENDS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_INPUT_VERSION_TEMPLATE)
        message(FATAL_ERROR "[GenerateAppRCFiles] Missing required argument: INPUT_VERSION_TEMPLATE")
    endif()
    if(NOT ARG_INPUT_JSON_TARGET)
        message(FATAL_ERROR "[GenerateAppRCFiles] Missing required argument: INPUT_JSON_TARGET")
    endif()
    if(NOT ARG_OUTPUT_FILE)
        message(FATAL_ERROR "[GenerateAppRCFiles] Missing required argument: OUTPUT_FILE")
    endif()
    if(NOT ARG_INPUT_JSON_FILE)
        message(FATAL_ERROR "[GenerateAppRCFiles] Missing required argument: INPUT_JSON_FILE")
    endif()

    add_custom_command(
        OUTPUT ${ARG_OUTPUT_FILE}
        COMMAND ${CMAKE_COMMAND} -DINPUT_JSON_FILE=${ARG_INPUT_JSON_FILE}
                                 -DINPUT_TEMPLATE_FILE=${ARG_INPUT_VERSION_TEMPLATE}
                                 -DFILE_DESCRIPTION=${ARG_FILE_DESCRIPTION}
                                 -DINTERNAL_NAME=${ARG_INTERNAL_NAME}
                                 -DORIGINAL_FILENAME=${ARG_ORIGINAL_FILENAME}
                                 -DOUTPUT_H=${ARG_OUTPUT_FILE}
                                 -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/GenerateAppRCFiles.cmake"
        COMMENT "Generating ${ARG_OUTPUT_FILE} from ${ARG_INPUT_JSON_FILE} using ${ARG_INPUT_VERSION_TEMPLATE}"
        DEPENDS ${ARG_INPUT_JSON_FILE}
    )
    
    get_filename_component(MODULE_NAME ${ARG_OUTPUT_FILE} NAME)
    set(CUSTOM_TARGET_NAME generate_${MODULE_NAME}_rc)
    add_custom_target(${CUSTOM_TARGET_NAME} ALL DEPENDS ${ARG_OUTPUT_FILE})
    set_target_properties(${CUSTOM_TARGET_NAME} PROPERTIES FOLDER codegen)
    add_dependencies(${CUSTOM_TARGET_NAME} ${ARG_INPUT_JSON_TARGET})

    list(LENGTH ARG_UNPARSED_ARGUMENTS unparsed_count)
    if(NOT unparsed_count EQUAL 1)
        message(FATAL_ERROR "函数调用错误: 需要指定1个输出变量名表示TARGET_NAME")
    endif()
    list(GET ARG_UNPARSED_ARGUMENTS 0 app_version_target)
    set(${app_version_target} ${CUSTOM_TARGET_NAME} PARENT_SCOPE)
endfunction()
