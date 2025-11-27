include_guard()

# ==============================
# 脚本模式：真正的生成逻辑
# ==============================
if(CMAKE_SCRIPT_MODE_FILE)
  if(NOT DEFINED INPUT_JSON)
    message(FATAL_ERROR "[NormalizeTokens] missing -DINPUT_JSON")
  endif()

  if(NOT DEFINED DIR_PREFIX)
    message(FATAL_ERROR "[NormalizeTokens] missing -DDIR_PREFIX")
  endif()

  if(NOT DEFINED OUTPUT_JSON)
    message(FATAL_ERROR "[NormalizeTokens] missing -DOUTPUT_JSON")
  endif()

  if(NOT EXISTS "${INPUT_JSON}")
    message(FATAL_ERROR "[NormalizeTokens] input JSON not found: ${INPUT_JSON}")
  endif()

  message(STATUS "[NormalizeTokens] Normalizing tokens from ${INPUT_JSON} with prefix ${DIR_PREFIX} to ${OUTPUT_JSON}")

  file(READ "${INPUT_JSON}" __json)

  # 简单 JSON 字符串转义用于回写
  function(_json_escape outVar text)
    set(s "${text}")
    string(REPLACE "\\" "\\\\" s "${s}")
    string(REPLACE "\"" "\\\"" s "${s}")
    string(REPLACE "\n" "\\n" s "${s}")
    set(${outVar} "${s}" PARENT_SCOPE)
  endfunction()

  # 处理一个分类（从 resources.images / resources.videos 里收集）
  #
  # 期待 JSON 结构：
  # {
  #   "resources": {
  #     "images": [ { "token": "...", "path": "..." }, ... ],
  #     "videos": [ { "token": "...", "path": "..." }, ... ]
  #   }
  # }
  #
  function(_collect_category jsonText dirPrefix category outListVar)
    # resources.<category> 的数组长度
    string(JSON catLen LENGTH "${jsonText}" resources ${category})
    if(catLen EQUAL 0)
      set(${outListVar} "" PARENT_SCOPE)
      return()
    endif()

    set(files "")
    set(seenAliases "")

    math(EXPR lastIndex "${catLen}-1")
    foreach(i RANGE ${lastIndex})
      # 取第 i 个元素（是一个 JSON 对象）
      string(JSON item GET "${jsonText}" resources ${category} ${i})

      # 解析 token 与 path（token 目前只参与逻辑，不写入 normalized JSON）
      string(JSON tok GET "${item}" token)
      string(JSON p   GET "${item}" path)

      # 补全绝对路径
      if(IS_ABSOLUTE "${p}")
        set(abs "${p}")
      else()
        file(TO_CMAKE_PATH "${dirPrefix}/${p}" abs)
      endif()

      # 可选：严格校验文件是否存在（如不想校验可注释掉）
      if(NOT EXISTS "${abs}")
        message(FATAL "[NormalizeTokens] not found: ${abs}")
      endif()

      # alias=文件名（同一分类内去重），保持和之前一致的逻辑
      get_filename_component(alias "${abs}" NAME)
      list(FIND seenAliases "${alias}" _dup)
      if(_dup EQUAL -1)
        list(APPEND seenAliases "${alias}")
        list(APPEND files "${alias}::${abs}")
      endif()
    endforeach()

    set(${outListVar} "${files}" PARENT_SCOPE)
  endfunction()

  # 收集 images / videos
  _collect_category("${__json}" "${DIR_PREFIX}" images files_images)
  _collect_category("${__json}" "${DIR_PREFIX}" videos files_videos)

    # 组装 normalized JSON
  # 输出结构：
  # {
  #   "resources": [
  #     { "prefix": "/images", "files": [ ... ] },
  #     { "prefix": "/videos", "files": [ ... ] }
  #   ]
  # }
  set(out "{\n  \"resources\": [\n")
  set(groupCount 0)

  # 一个小工具函数，用来把 files_<category> 拼成 JSON
  function(_append_group_json outVar prefix filesVar)
    set(out_local "${${outVar}}")
    set(listVar "${${filesVar}}")

    if(NOT listVar)
      set(${outVar} "${out_local}" PARENT_SCOPE)
      return()
    endif()

    # 如果已经有前一组了，加逗号换行
    if(NOT "${out_local}" STREQUAL "{\n  \"resources\": [\n")
      string(APPEND out_local ",\n")
    endif()

    string(APPEND out_local "    { \"prefix\": \"${prefix}\", \"files\": [\n")

    list(LENGTH listVar n)
    math(EXPR last "${n}-1")
    foreach(idx RANGE ${last})
      list(GET listVar ${idx} pair)
      string(REPLACE "::" ";" kv "${pair}")
      list(GET kv 0 alias)
      list(GET kv 1 path)

      _json_escape(aliasEsc "${alias}")
      _json_escape(pathEsc  "${path}")

      string(APPEND out_local "      { \"alias\": \"${aliasEsc}\", \"path\": \"${pathEsc}\" }")
      if(NOT idx EQUAL last)
        string(APPEND out_local ",\n")
      else()
        string(APPEND out_local "\n")
      endif()
    endforeach()

    string(APPEND out_local "    ] }")
    set(${outVar} "${out_local}" PARENT_SCOPE)
  endfunction()

  # 先 images 再 videos，前缀写死，避免判断错误
  _append_group_json(out "/images" files_images)
  _append_group_json(out "/videos" files_videos)

  string(APPEND out "\n  ]\n}\n")



  get_filename_component(outDir "${OUTPUT_JSON}" DIRECTORY)
  file(MAKE_DIRECTORY "${outDir}")
  file(WRITE "${OUTPUT_JSON}" "${out}")

  message(STATUS "[NormalizeTokens] Generated ${OUTPUT_JSON}")
endif()

# ==========================================
# 函数：外部工程调用（创建命令 + 目标）
# 用法：
#   generate_normalized_tokens(
#     INPUT_JSON <file>
#     DIR_PREFIX <dir>         # 可选，默认 ${CMAKE_SOURCE_DIR}
#     OUTPUT_JSON <file>
#     out_target_var           # 最后一个位置参数：返回目标名
#   )
# ==========================================
function(generate_normalized_tokens)
  set(oneValueArgs
    INPUT_JSON
    DIR_PREFIX
    OUTPUT_JSON
  )
  cmake_parse_arguments(NORM "" "${oneValueArgs}" "" ${ARGN})

  if(NOT NORM_OUTPUT_JSON)
    message(FATAL_ERROR "[GenerateNormalizedTokens] OUTPUT_JSON is required")
  endif()
  
  if(NOT NORM_INPUT_JSON)
    message(FATAL_ERROR "[GenerateNormalizedTokens] INPUT_JSON is required")
  endif()

  if(NOT NORM_DIR_PREFIX)
    message(FATAL_ERROR "[GenerateNormalizedTokens] DIR_PREFIX is required")
  endif()

  # 自定义命令：调用本脚本的脚本模式
  add_custom_command(
    OUTPUT "${NORM_OUTPUT_JSON}"
    COMMAND ${CMAKE_COMMAND}
            -DINPUT_JSON=${NORM_INPUT_JSON}
            -DDIR_PREFIX=${NORM_DIR_PREFIX}
            -DOUTPUT_JSON=${NORM_OUTPUT_JSON}
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/NormalizeTokens.cmake"
    DEPENDS "${NORM_INPUT_JSON}"
    COMMENT "[GenerateNormalizedTokens] ${NORM_OUTPUT_JSON}"
    VERBATIM
  )

  # 包装成一个 ALL 目标
  set(tgt generate_normalized_tokens)
  add_custom_target(${tgt} ALL DEPENDS "${NORM_OUTPUT_JSON}")
  set_target_properties(${tgt} PROPERTIES FOLDER codegen)

  # 把目标名回传给调用点（最后一个未解析实参）
  list(LENGTH NORM_UNPARSED_ARGUMENTS _n)
  if(NOT _n EQUAL 1)
    message(FATAL_ERROR "[GenerateNormalizedTokens] 需要 1 个位置参数作为返回的目标变量名")
  endif()
  list(GET NORM_UNPARSED_ARGUMENTS 0 _retvar)
  set(${_retvar} ${tgt} PARENT_SCOPE)
endfunction()
