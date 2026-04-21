
set(RESOURCE_IDE_FOLDER commonhead/codegen)
set(RESOURCE_OUT_PUT_DIR ${UCF_CODE_GENERATOR_DIR}/UIResourceAsset)

set(RCC_FILES)
set(RCC_TARGETS)
message(STATUS "GLOBAL_ASSET_THEME_FILES: ${GLOBAL_ASSET_THEME_FILES}")
foreach(item IN LISTS GLOBAL_ASSET_THEME_FILES)

  get_filename_component(FILENAME_NO_EXT ${item} NAME_WE)
  set(RCC_OUT_PUT_FILE UIAssetResource_${FILENAME_NO_EXT}.qrc)
  set(RCC_OUTPUT_PATH ${RESOURCE_OUT_PUT_DIR}/${RCC_OUT_PUT_FILE})
  set(RCC_TEMEPLATE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/templates/UIAssetResource.qrc.j2)

  generate_from_template(
      TEMPLATE_FILE ${RCC_TEMEPLATE_FILE}
      INPUT_FILE ${item}
      OUTPUT_FILE ${RCC_OUTPUT_PATH}
      GFT_RCC_TARGET
      EXTRA_PARAMS "base_dir=${GLOBAL_ASSET_BASE_DIR}"
  )
  list(APPEND RCC_FILES ${RCC_OUTPUT_PATH})
  list(APPEND RCC_TARGETS ${GFT_RCC_TARGET})
endforeach()

set(Resource_RCC_Target ResourceRCC)
add_library(${Resource_RCC_Target} OBJECT)
set_target_properties(${Resource_RCC_Target} PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  AUTORCC ON
  AUTOMOC OFF
  AUTOUIC OFF
  FOLDER ${RESOURCE_IDE_FOLDER}
)
target_link_libraries(${Resource_RCC_Target} PRIVATE Qt6::Core)
target_sources(${Resource_RCC_Target} PRIVATE ${RCC_FILES})
add_dependencies(${Resource_RCC_Target} ${RCC_TARGETS})

# INTERFACE library wraps the OBJECT RCC lib so consumers only need to know
# about UIResourceAssetRCCInterface.
# Linking an OBJECT library via target_link_libraries (CMake 3.12+)
# automatically propagates the .obj files AND creates a proper build
# dependency — no MSBuild race condition, no stripped resource initializers.
add_library(UIResourceAssetRCCInterface INTERFACE)
target_link_libraries(UIResourceAssetRCCInterface INTERFACE ${Resource_RCC_Target})

