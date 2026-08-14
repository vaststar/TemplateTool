include(NormalizeTokens.cmake)

set(AssetResourceDir "${CMAKE_CURRENT_SOURCE_DIR}/../data")
cmake_path(NORMAL_PATH AssetResourceDir)
set(rawJson "${AssetResourceDir}/themes/normal.json")

if(NOT IS_DIRECTORY "${AssetResourceDir}")
  message(FATAL_ERROR "[ResourceAssetLoader] asset data directory not found: ${AssetResourceDir}")
endif()

if(NOT EXISTS "${rawJson}")
  message(FATAL_ERROR "[ResourceAssetLoader] theme definition not found: ${rawJson}")
endif()
set(GLOBAL_ASSET_NORMALIZED_JSON_FILE ${UCF_CODE_GENERATOR_DIR}/ResourceAsset/normalized_tokens_normal.json CACHE INTERNAL "Global normalize asset json filePath")

generate_normalized_tokens(
  INPUT_JSON   "${rawJson}"
  DIR_PREFIX   "${AssetResourceDir}"
  OUTPUT_JSON  "${GLOBAL_ASSET_NORMALIZED_JSON_FILE}"
  norm_tokens_target
)
set(GLOBAL_ASSET_NORMALIZED_JSON_TARGET ${norm_tokens_target} CACHE INTERNAL "Global normalize asset json target")
