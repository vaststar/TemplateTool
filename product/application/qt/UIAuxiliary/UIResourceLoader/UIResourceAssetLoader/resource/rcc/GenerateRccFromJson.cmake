# ResourceFromJson.cmake

include_guard()

function(generate_resource_object_from_json _retvar)
  set(oneValueArgs
    INPUT_JSON     # 原始资源 json
    DIR_PREFIX     # 资源根目录
    OUTPUT_QRC     # 相对 binary dir 的 qrc 路径，比如 "gen/normal.qrc"
    NORM_TARGET    # 可选：如果有“生成 normalized json”的 target，就填它
  )
  cmake_parse_arguments(GR "" "${oneValueArgs}" "" ${ARGN})

  if(NOT GR_INPUT_JSON)
    message(FATAL_ERROR "[generate_resource_object_from_json] INPUT_JSON is required")
  endif()
  if(NOT GR_OUTPUT_QRC)
    message(FATAL_ERROR "[generate_resource_object_from_json] OUTPUT_QRC is required (relative)")
  endif()
  if(NOT GR_DIR_PREFIX)
    set(GR_DIR_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}")
  endif()

  # 绝对 qrc 路径给脚本用
  set(_qrc_rel "${GR_OUTPUT_QRC}")
  set(_qrc_abs "${CMAKE_CURRENT_BINARY_DIR}/${_qrc_rel}")

  # 第一步：json -> qrc
  add_custom_command(
    OUTPUT  "${_qrc_rel}"    # 相对路径，AUTORCC / Qt 都开心
    COMMAND ${CMAKE_COMMAND}
            -DINPUT_JSON=${GR_INPUT_JSON}
            -DDIR_PREFIX=${GR_DIR_PREFIX}
            -DOUTPUT_QRC=${_qrc_abs}
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ResObjBuildtime.cmake"  # 里面写解析 json 写 qrc 的脚本模式
    DEPENDS "${GR_INPUT_JSON}"
    COMMENT "[ResObj] ${_qrc_abs}"
    VERBATIM
  )

  # 第二步：qrc -> rcc C++（交给 AUTORCC）
  string(MAKE_C_IDENTIFIER "${_qrc_rel}" _obj_name_hint)
  set(_obj "ResObj_${_obj_name_hint}")

  add_library(${_obj} OBJECT)
  set_target_properties(${_obj} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    AUTORCC ON
    AUTOMOC OFF
    AUTOUIC OFF
    FOLDER "resources"
  )
  target_link_libraries(${_obj} PRIVATE Qt6::Core)

  target_sources(${_obj} PRIVATE "${_qrc_rel}")

  # 如果有上游 normalize target，就显式依赖
  if(GR_NORM_TARGET)
    add_dependencies(${_obj} ${GR_NORM_TARGET})
  endif()

  # 把 OBJECT target 名返回给调用者
  set(${_retvar} ${_obj} PARENT_SCOPE)
endfunction()
