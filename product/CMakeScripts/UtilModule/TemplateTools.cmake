# === TemplateTools.cmake ===
# 提供 generate_from_template() 函数，用于从模板 + JSON 输入生成文件。
#
# 用法示例：
# generate_from_template(
#     TEMPLATE_FILE  ${CMAKE_CURRENT_SOURCE_DIR}/templates/Foo.cpp.jinja
#     INPUT_FILE     ${CMAKE_CURRENT_SOURCE_DIR}/data/foo.json
#     OUTPUT_FILE    ${CMAKE_CURRENT_BINARY_DIR}/generated/Foo.cpp
#     DEPENDS        ${CMAKE_CURRENT_SOURCE_DIR}/tools/shared_utils.py  # 可选
# )
include_guard()

# 核心函数：创建虚拟环境（响应requirements.txt变更）
function(jinja_create_venv)
    set(options)  # 没有布尔选项
    set(oneValueArgs VENV_TARGET_NAME)
    set(multiValueArgs)
    cmake_parse_arguments(JCV "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(VENV_DIR "${CMAKE_BINARY_DIR}/jinja_venv")
    set(REQUIREMENT_FILE "${CMAKE_SOURCE_DIR}/product/codegen/template_render/requirements.txt")
    message(STATUS "[jinja_create_venv] requirements path: ${REQUIREMENT_FILE}")
    set(MARKER_FILE "${VENV_DIR}/.venv_ready")  # 环境就绪标记

    if(WIN32)
        set(PIP_PATH "${VENV_DIR}/Scripts/pip.exe")
    else()
        set(PIP_PATH "${VENV_DIR}/bin/pip")
    endif()
    message(STATUS "[jinja_create_venv] pip path: ${PIP_PATH}")

    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    message(STATUS "[generate_from_template] python3 path: ${Python3_EXECUTABLE}")

    # 定义虚拟环境构建命令
    add_custom_command(
        OUTPUT ${MARKER_FILE}
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${VENV_DIR}           # 清理旧环境
        COMMAND ${Python3_EXECUTABLE} -m venv ${VENV_DIR}        # 创建新环境
        COMMAND ${PIP_PATH} install -r ${REQUIREMENT_FILE}   # 安装依赖
        COMMAND ${CMAKE_COMMAND} -E touch ${MARKER_FILE}         # 创建标记
        DEPENDS ${REQUIREMENT_FILE} 
        COMMENT "Building Jinja2 venv with dependencies"
    )

    # 声明目标以驱动环境创建
    add_custom_target(${JCV_VENV_TARGET_NAME} 
        DEPENDS ${MARKER_FILE}
    )
    
    # 返回Python解释器路径（跨平台处理）
    if(WIN32)
        set(VENV_PYTHON_PATH "${VENV_DIR}/Scripts/python.exe" PARENT_SCOPE)
    else()
        set(VENV_PYTHON_PATH "${VENV_DIR}/bin/python" PARENT_SCOPE)
    endif()
endfunction()


function(generate_from_template)
    set(options)  # 没有布尔选项
    set(oneValueArgs TEMPLATE_FILE INPUT_FILE OUTPUT_FILE IDE_FOLDER)
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
    
    get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    set(VENV_TARGET "venv_ready_${MODULE_NAME}")
    jinja_create_venv(VENV_TARGET_NAME ${VENV_TARGET})

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
            ${VENV_TARGET}
        COMMENT "Generating ${GFT_OUTPUT_FILE} from ${GFT_TEMPLATE_FILE} using ${GFT_INPUT_FILE}"
        VERBATIM
    )
endfunction()
