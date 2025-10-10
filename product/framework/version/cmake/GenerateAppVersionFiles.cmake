# cmake/GenerateAppVersionFiles.cmake

# 参数：
# -DVERSION_META_FILE
# -DINPUT_VERSION_TEMPLATE
# -DINPUT_RC_TEMPLATE
# -DOUTPUT_VERSION_H
# -DOUTPUT_VERSION_RC

file(READ "${VERSION_META_FILE}" META_CONTENT)

macro(json_extract key var)
    string(REGEX MATCH "\"${key}\":[ ]*\"([^\"]+)\"" _ "${META_CONTENT}")
    set(${var} "${CMAKE_MATCH_1}")
endmacro()

json_extract("PROJECT_VERSION_MAJOR" PROJECT_VERSION_MAJOR)
json_extract("PROJECT_VERSION_MINOR" PROJECT_VERSION_MINOR)
json_extract("PROJECT_VERSION_PATCH" PROJECT_VERSION_PATCH)
json_extract("GIT_COMMIT_HASH" GIT_COMMIT_HASH)
json_extract("GIT_CURRENT_BRANCH" GIT_CURRENT_BRANCH)
json_extract("PROJECT_COMPILE_TIME" PROJECT_COMPILE_TIME)

# 生成 version.h
configure_file("${INPUT_VERSION_TEMPLATE}" "${OUTPUT_VERSION_H}" @ONLY)

# rc 模板依赖 version.h 路径
get_filename_component(version_h_name "${OUTPUT_VERSION_H}" NAME)
set(version_h_file "${version_h_name}")

configure_file("${INPUT_RC_TEMPLATE}" "${OUTPUT_VERSION_RC}" @ONLY)

message(STATUS "[GenerateAppVersionFiles] Generated:")
message(STATUS "  ${OUTPUT_VERSION_H}")
message(STATUS "  ${OUTPUT_VERSION_RC}")
