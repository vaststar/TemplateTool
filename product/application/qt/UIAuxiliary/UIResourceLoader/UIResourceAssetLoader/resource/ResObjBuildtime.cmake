include_guard()

# ─────────────────────────────────────────────────────────────
# 脚本模式：从 normalized.json 直接生成 .qrc（不落地中间文件）
# 用法：
#   cmake -DINPUT_JSON=<normalized.json> -DOUTPUT_QRC=<out.qrc> -P ResObjBuildtime.cmake
# normalized.json 结构：
# {
#   "resources":[
#     {"prefix":"/images","files":[{"alias":"a.png","path":"/abs/..."}, ...]},
#     ...
#   ]
# }
# ─────────────────────────────────────────────────────────────
if(CMAKE_SCRIPT_MODE_FILE)
  if(NOT DEFINED INPUT_JSON OR NOT DEFINED OUTPUT_QRC)
    message(FATAL_ERROR "[ResObj] need -DINPUT_JSON and -DOUTPUT_QRC")
  endif()
  # 保险：去掉可能被传入的包裹引号
  string(REGEX REPLACE "^\"|\"$" "" INPUT_JSON  "${INPUT_JSON}")
  string(REGEX REPLACE "^\"|\"$" "" OUTPUT_QRC  "${OUTPUT_QRC}")

  if(NOT EXISTS "${INPUT_JSON}")
    message(FATAL_ERROR "[ResObj] INPUT_JSON not found: ${INPUT_JSON}")
  endif()

  file(READ "${INPUT_JSON}" _json)
  string(JSON _glen LENGTH "${_json}" resources)

  set(_xml "<RCC>\n")
  if(NOT _glen EQUAL 0)
    math(EXPR _glast "${_glen}-1")
    foreach(i RANGE ${_glast})
      string(JSON _prefix   GET    "${_json}" resources ${i} prefix)
      string(JSON _flen     LENGTH "${_json}" resources ${i} files)

      string(APPEND _xml "  <qresource prefix=\"${_prefix}\">\n")
      if(NOT _flen EQUAL 0)
        math(EXPR _flast "${_flen}-1")
        foreach(j RANGE ${_flast})
          string(JSON _alias GET "${_json}" resources ${i} files ${j} alias)
          string(JSON _path  GET "${_json}" resources ${i} files ${j} path)
          string(APPEND _xml "    <file alias=\"${_alias}\">${_path}</file>\n")
        endforeach()
      endif()
      string(APPEND _xml "  </qresource>\n")
    endforeach()
  endif()
  string(APPEND _xml "</RCC>\n")

  get_filename_component(_dir "${OUTPUT_QRC}" DIRECTORY)
  file(MAKE_DIRECTORY "${_dir}")
  file(WRITE "${OUTPUT_QRC}" "${_xml}")
  message(STATUS "[ResObj] QRC -> ${OUTPUT_QRC}")
endif()

# ─────────────────────────────────────────────────────────────
# 对外函数（构建期，一次命令）：
#   normalized.json --(1个 add_custom_command)--> .qrc --(AUTORCC)--> OBJECT
#
# 用法：
#   generate_resource_object_buildtime_from_normalized(
#     INPUT_JSON   <normalized.json>
#     NORM_TARGET  <step1 target>     # 可选但推荐：保证构建顺序
#     OBJECT_NAME  <name>             # 可选；默认基于 JSON 路径生成唯一名
#     <ret_object_var>                # 返回 OBJECT 目标名
#   )
# ─────────────────────────────────────────────────────────────
function(generate_resource_object_buildtime_from_normalized)
  set(one INPUT_JSON NORM_TARGET OBJECT_NAME)
  cmake_parse_arguments(GR "" "${one}" "" ${ARGN})

  # 返回变量名
  list(LENGTH GR_UNPARSED_ARGUMENTS _n)
  if(NOT _n EQUAL 1)
    message(FATAL_ERROR "[ResObj] need 1 positional arg as return var (OBJECT target)")
  endif()
  list(GET GR_UNPARSED_ARGUMENTS 0 _retvar)

  if(NOT GR_INPUT_JSON)
    message(FATAL_ERROR "[ResObj] INPUT_JSON (normalized.json) is required")
  endif()

  # 绝对路径
  get_filename_component(_abs_json "${GR_INPUT_JSON}" ABSOLUTE)

  # 名字/路径
  if(GR_OBJECT_NAME)
    set(_obj "${GR_OBJECT_NAME}")
  else()
    string(MD5 _h "${_abs_json}")
    set(_obj "ResObj_${_h}")
  endif()
  set(_script "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ResObjBuildtime.cmake")
  set(_qrc    "${CMAKE_CURRENT_BINARY_DIR}/gen/${_obj}.qrc")

  # ★ 仅一个构建期命令：JSON -> QRC（无中间文件）
  add_custom_command(
    OUTPUT  "${_qrc}"
    COMMAND "${CMAKE_COMMAND}"
            -DINPUT_JSON:PATH=${_abs_json}
            -DOUTPUT_QRC:PATH=${_qrc}
            -P "${_script}"
    # 脚本或 JSON 改动会触发重建
    DEPENDS "${_abs_json}" "${_script}"
    COMMENT "[ResObj] QRC -> ${_qrc}"
    VERBATIM
  )
  set_source_files_properties("${_qrc}" PROPERTIES GENERATED TRUE)

  # OBJECT + AUTORCC
  find_package(Qt6 6.5 REQUIRED COMPONENTS Core)
  add_library(${_obj} OBJECT)
  set_target_properties(${_obj} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    AUTORCC ON
    AUTOMOC OFF
    AUTOUIC OFF
    FOLDER "resources"
  )
  target_link_libraries(${_obj} PRIVATE Qt6::Core)
  target_sources(${_obj} PRIVATE "${_qrc}")

  # 若提供了第一步目标，则显式顺序
  if(GR_NORM_TARGET)
    add_dependencies(${_obj} ${GR_NORM_TARGET})
  endif()

  # 返回 OBJECT 名
  set(${_retvar} ${_obj} PARENT_SCOPE)
endfunction()
