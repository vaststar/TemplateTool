include(NormalizeTokens.cmake)

set(AssetResourceDir "${CMAKE_CURRENT_SOURCE_DIR}/../resource")
set(rawJson   "${AssetResourceDir}/themes/normal.json")
set(GLOBAL_ASSET_NORMALIZED_JSON_FILE ${UCF_CODE_GENERATOR_DIR}/ResourceAsset/normalized_tokens_normal.json CACHE INTERNAL "Global normalize asset json filePath")

generate_normalized_tokens(
  INPUT_JSON   "${rawJson}"
  DIR_PREFIX   "${AssetResourceDir}"
  OUTPUT_JSON  "${GLOBAL_ASSET_NORMALIZED_JSON_FILE}"
  norm_tokens_target
)
set(GLOBAL_ASSET_NORMALIZED_JSON_TARGET ${norm_tokens_target} CACHE INTERNAL "Global normalize asset json target")

