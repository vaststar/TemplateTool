include("ResObjBuildtime.cmake")

# 3) 直接用 qt_add_resources 生成 OBJECT（不落地 .qrc）
generate_resource_object_buildtime_from_normalized(
  INPUT_JSON  "${GLOBAL_ASSET_NORMALIZED_JSON_FILE}"
  NORM_TARGET "${GLOBAL_ASSET_NORMALIZED_JSON_TARGET}"   # 可省略
  OBJECT_NAME "ResPackObj"              # 可自定义，也可不传
  resobj_tgt                           # ← 返回 OBJECT 目标名
)

add_library(ResPack INTERFACE)
target_sources(ResPack INTERFACE $<TARGET_OBJECTS:${resobj_tgt}>)
