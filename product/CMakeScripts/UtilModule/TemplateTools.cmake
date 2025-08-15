# === TemplateTools.cmake ===
# 提供 generate_from_template() 函数，用于从模板 + JSON 输入生成文件。
include_guard()

# 核心函数：创建虚拟环境（响应requirements.txt变更）
function(jinja_create_venv)
    set(VENV_DIR "${CMAKE_BINARY_DIR}/jinja_venv")
    set(REQUIREMENT_FILE "${CMAKE_SOURCE_DIR}/product/codegen/template_render/requirements.txt")
    message(STATUS "[jinja_create_venv] requirements path: ${REQUIREMENT_FILE}")
    set(MARKER_FILE "${VENV_DIR}/.venv_ready")  # 环境就绪标记


    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    message(STATUS "[jinja_create_venv] python3 path: ${Python3_EXECUTABLE}")

    # 返回Python解释器路径（跨平台处理）
    if(WIN32)
        set(PIP_PATH "${VENV_DIR}/Scripts/pip.exe")
        set(PYTHON_PATH "${VENV_DIR}/Scripts/python.exe")
    else()
        set(PIP_PATH "${VENV_DIR}/bin/pip")
        set(PYTHON_PATH "${VENV_DIR}/bin/python")
    endif()

    add_custom_command(
        OUTPUT ${MARKER_FILE}
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${VENV_DIR}           # 清理旧环境
        COMMAND ${Python3_EXECUTABLE} -m venv ${VENV_DIR}        # 创建新环境
        COMMAND ${CMAKE_COMMAND} -E sleep 1                      # 解决权限问题：安装前等待文件系统就绪
        COMMAND ${PIP_PATH} install -r ${REQUIREMENT_FILE}       # 安装依赖
        COMMAND ${CMAKE_COMMAND} -E touch ${MARKER_FILE}         # 创建标记
        COMMENT "Building Jinja2 venv with dependencies, create virtual env and install requirements: ${REQUIREMENT_FILE}"
        VERBATIM
    )

    set (VENV_NAME "jinja_venv")
    if (NOT TARGET ${VENV_NAME})
        add_custom_target(${VENV_NAME} 
            DEPENDS ${MARKER_FILE}
        )
        set_target_properties(${VENV_NAME} PROPERTIES FOLDER codegen)
    endif()

    set(VENV_PIP_PATH ${PIP_PATH} PARENT_SCOPE)
    set(VENV_PYTHON_PATH ${PYTHON_PATH} PARENT_SCOPE)
    set(VENV_TARGET_NAME ${VENV_NAME} PARENT_SCOPE)
endfunction()


function(generate_from_template)
    set(options)  # 没有布尔选项
    set(oneValueArgs TEMPLATE_FILE INPUT_FILE OUTPUT_FILE)
    set(multiValueArgs DEPENDS)
    cmake_parse_arguments(GFT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # === 参数检查 ===
    if(NOT GFT_TEMPLATE_FILE)
        message(FATAL_ERROR "[generate_from_template] Missing required argument: TEMPLATE_FILE")
    endif()
    if(NOT GFT_INPUT_FILE)
        message(FATAL_ERROR "[generate_from_template] Missing required argument: INPUT_FILE")
    endif()
    if(NOT GFT_OUTPUT_FILE)
        message(FATAL_ERROR "[generate_from_template] Missing required argument: OUTPUT_FILE")
    endif()

    jinja_create_venv()
    message(STATUS "[generate_from_template] pip path: ${VENV_PIP_PATH}, python path: ${VENV_PYTHON_PATH}")

    set(SCRIPT_PATH "${CMAKE_SOURCE_DIR}/product/codegen/template_render/render_template.py")
    message(STATUS "[generate_from_template] render script path: ${SCRIPT_PATH}")

    # === 路径准备 ===
    get_filename_component(_output_dir "${GFT_OUTPUT_FILE}" DIRECTORY)
    file(MAKE_DIRECTORY "${_output_dir}")

    message(STATUS "[generate_from_template] Generating '${GFT_OUTPUT_FILE}' from template '${GFT_TEMPLATE_FILE}' using input '${GFT_INPUT_FILE}'")
    # === 构建命令 ===
    add_custom_command(
        OUTPUT "${GFT_OUTPUT_FILE}"
        COMMAND ${VENV_PYTHON_PATH}
                ${SCRIPT_PATH}
                --template "${GFT_TEMPLATE_FILE}"
                --input "${GFT_INPUT_FILE}"
                --output "${GFT_OUTPUT_FILE}"
        DEPENDS
            "${GFT_TEMPLATE_FILE}"
            "${GFT_INPUT_FILE}"
            ${GFT_DEPENDS}
            ${SCRIPT_PATH}
            ${VENV_TARGET_NAME}
        COMMENT "Generating ${GFT_OUTPUT_FILE} from ${GFT_TEMPLATE_FILE} using ${GFT_INPUT_FILE}"
        VERBATIM
    )
    
    list(LENGTH GFT_UNPARSED_ARGUMENTS unparsed_count)
    if(NOT unparsed_count EQUAL 1)
        message(FATAL_ERROR "函数调用错误: 需要指定1个输出变量名表示TARGET_NAME")
    endif()
    list(GET GFT_UNPARSED_ARGUMENTS 0 generate_from_template_target)

    get_filename_component(MODULE_NAME ${GFT_OUTPUT_FILE} NAME)
    set(MODULE_TARGET_NAME generate_${MODULE_NAME})
    add_custom_target(${MODULE_TARGET_NAME} ALL DEPENDS ${GFT_OUTPUT_FILE})
    set_target_properties(${MODULE_TARGET_NAME} PROPERTIES FOLDER codegen)
    set(${generate_from_template_target} ${MODULE_TARGET_NAME} PARENT_SCOPE)
endfunction()
