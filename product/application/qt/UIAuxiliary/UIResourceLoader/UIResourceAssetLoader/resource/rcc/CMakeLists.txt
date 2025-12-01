# 加载资源生成逻辑
include(${CMAKE_CURRENT_SOURCE_DIR}/GenerateRccFromJson.cmake)

# 生成 .qrc 的 target
generate_resource_object_from_json(
    ResObjTarget
    INPUT_JSON   ${GLOBAL_ASSET_RESOURCE_FOLDER}/themes/normal.json
    DIR_PREFIX   ${GLOBAL_ASSET_RESOURCE_FOLDER}/assets
    OUTPUT_QRC   ${UCF_CODE_GENERATOR_DIR}/ResourceAsset/rcc/normal.qrc
)

#
# 定义 INTERFACE 库，把 .qrc 编译进消费者 target
#
add_library(ResourcePack INTERFACE)
target_link_libraries(ResourcePack INTERFACE ${ResObjTarget})

