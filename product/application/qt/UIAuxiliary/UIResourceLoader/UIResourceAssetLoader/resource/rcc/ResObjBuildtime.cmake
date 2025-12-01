# ResObjBuildtime.cmake
include_guard()

if(NOT CMAKE_SCRIPT_MODE_FILE)
  # 被正常 include 时不做事
  return()
endif()

# 必要参数检查
if(NOT DEFINED INPUT_JSON)
  message(FATAL_ERROR "[ResObj] missing -DINPUT_JSON")
endif()
if(NOT DEFINED OUTPUT_QRC)
  message(FATAL_ERROR "[ResObj] missing -DOUTPUT_QRC")
endif()
if(NOT DEFINED DIR_PREFIX)
  set(DIR_PREFIX "${CMAKE_SOURCE_DIR}")
endif()

if(NOT EXISTS "${INPUT_JSON}")
  message(FATAL_ERROR "[ResObj] INPUT_JSON not found: ${INPUT_JSON}")
endif()

message(STATUS "[ResObj] Generating RCC from ${INPUT_JSON} to ${OUTPUT_QRC}")

file(READ "${INPUT_JSON}" __json)

# 小工具：拼 qresource 段
function(_append_qresource outVar jsonText dirPrefix category prefix)
  # 计算 resources.<category> 数组长度
  string(JSON count LENGTH "${jsonText}" resources ${category})
  if(count EQUAL 0)
    return()
  endif()

  set(out_local "${${outVar}}")

  string(APPEND out_local "  <qresource prefix=\"${prefix}\">\n")

  math(EXPR last "${count}-1")
  foreach(i RANGE ${last})
    string(JSON item GET "${jsonText}" resources ${category} ${i})
    string(JSON token GET "${item}" token)
    string(JSON path  GET "${item}" path)

    # 绝对/相对路径处理
    if(IS_ABSOLUTE "${path}")
      set(abs "${path}")
    else()
      file(TO_CMAKE_PATH "${dirPrefix}/${path}" abs)
    endif()

    # 这里可以选做存在性检查
    if(NOT EXISTS "${abs}")
      message(WARNING "[ResObj] file not found: ${abs}")
    endif()

    # alias 用 token
    string(APPEND out_local "    <file alias=\"${token}\">${abs}</file>\n")
  endforeach()

  string(APPEND out_local "  </qresource>\n")

  set(${outVar} "${out_local}" PARENT_SCOPE)
endfunction()

set(qrc "<RCC>\n")

# images -> /images
_append_qresource(qrc "${__json}" "${DIR_PREFIX}" images "/images")

# videos -> /videos
_append_qresource(qrc "${__json}" "${DIR_PREFIX}" videos "/videos")

string(APPEND qrc "</RCC>\n")

# 确保目录存在
get_filename_component(outDir "${OUTPUT_QRC}" DIRECTORY)
file(MAKE_DIRECTORY "${outDir}")
file(WRITE "${OUTPUT_QRC}" "${qrc}")

message(STATUS "[ResObj] Generated ${OUTPUT_QRC}")
