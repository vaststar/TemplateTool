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

    set(SCRIPT_PATH "${CMAKE_SOURCE_DIR}/product/codegen/template_render/render_template.py")
    # === 路径准备 ===
    get_filename_component(_output_dir "${GFT_OUTPUT_FILE}" DIRECTORY)
    file(MAKE_DIRECTORY "${_output_dir}")
    message(STATUS "[generate_from_template] render_template path: ${SCRIPT_PATH}")
    message(STATUS "[generate_from_template] Generating '${GFT_OUTPUT_FILE}' from template '${GFT_TEMPLATE_FILE}' using input '${GFT_INPUT_FILE}'")
    # === 构建命令 ===
    add_custom_command(
        OUTPUT "${GFT_OUTPUT_FILE}"
        COMMAND ${Python3_EXECUTABLE}
                ${SCRIPT_PATH}
                --template "${GFT_TEMPLATE_FILE}"
                --input "${GFT_INPUT_FILE}"
                --output "${GFT_OUTPUT_FILE}"
        DEPENDS
            "${GFT_TEMPLATE_FILE}"
            "${GFT_INPUT_FILE}"
            ${GFT_DEPENDS}
            ${SCRIPT_PATH}
        COMMENT "Generating ${GFT_OUTPUT_FILE} from ${GFT_TEMPLATE_FILE} using ${GFT_INPUT_FILE}"
        VERBATIM
    )
endfunction()
