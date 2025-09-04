include(TemplateTools)
include(CapitalizeString)
# 设置资源字符串 JSON 文件路径

set(RESOURCE_IDE_FOLDER commonhead/codegen)
set(RESOURCE_OUT_PUT_DIR ${UCF_CODE_GENERATOR_DIR}/ResourceColor)

set(RESOURCE_TARGETS )

## ColorPalette
set(COLOR_JSON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/resource/palette.json)
set(COLOR_OUT_PUT_FILE ColorPaletteModel.h)
set(COLOR_OUTPUT_PATH ${RESOURCE_OUT_PUT_DIR}/${COLOR_OUT_PUT_FILE})
set(COLOR_TEMEPLATE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/templates/ColorPaletteModel.h.j2)

generate_from_template(
    TEMPLATE_FILE ${COLOR_TEMEPLATE_FILE}
    INPUT_FILE ${COLOR_JSON_PATH}
    OUTPUT_FILE ${COLOR_OUTPUT_PATH}
    COLOR_TARGET
)
list(APPEND RESOURCE_TARGETS ${COLOR_TARGET})

## ColorToken
set(COLOR_TOKEN_JSON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/resource/token.json  CACHE STRING "Path to token.json")
set(TOKEN_OUT_PUT_FILE ColorToken.h)
set(TOKEN_OUTPUT_PATH ${RESOURCE_OUT_PUT_DIR}/${TOKEN_OUT_PUT_FILE})
set(TOKEN_TEMEPLATE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/templates/ColorToken.h.j2)

generate_from_template(
    TEMPLATE_FILE ${TOKEN_TEMEPLATE_FILE}
    INPUT_FILE ${COLOR_TOKEN_JSON_PATH}
    OUTPUT_FILE ${TOKEN_OUTPUT_PATH}
    TOKEN_TARGET
)
list(APPEND RESOURCE_TARGETS ${TOKEN_TARGET})


## BuildThemeColorSet
set(THEME_JSON_NAMES "light" "dark")
foreach(item IN LISTS THEME_JSON_NAMES)
set(THEME_JSON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/resource/themes/${item}.json)
capitalize(${item} capital_json_name)
set(THEME_OUT_PUT_FILE Build${capital_json_name}ThemeColorSet.h)
set(THEME_OUTPUT_PATH ${RESOURCE_OUT_PUT_DIR}/${THEME_OUT_PUT_FILE})
set(THEME_TEMEPLATE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/templates/BuildThemeColorSet.h.j2)

generate_from_template(
    TEMPLATE_FILE ${THEME_TEMEPLATE_FILE}
    INPUT_FILE ${THEME_JSON_PATH}
    OUTPUT_FILE ${THEME_OUTPUT_PATH}
    THEME_TARGET
)
list(APPEND RESOURCE_TARGETS ${THEME_TARGET})
endforeach()


include(BuildInterface)
BuildInterface(
    MODULE_NAME 
        ResourceColor
    TARGET_SOURCE_HEADER_BASE_DIR
        ${RESOURCE_OUT_PUT_DIR}
    TARGET_SOURCE_PUBLIC_HEADER
        ${RESOURCE_OUTPUT_PATH}
    TARGET_DEPENDENCIES_PUBLIC
        ResourceColorLoaderModel
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${RESOURCE_OUT_PUT_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        ResourceColor
    IDE_FOLDER
        ${RESOURCE_IDE_FOLDER}
)
add_dependencies(ResourceColor ${RESOURCE_TARGETS})

