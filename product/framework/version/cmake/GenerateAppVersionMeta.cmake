# cmake/GenerateAppVersionMeta.cmake
include("${CMAKE_CURRENT_LIST_DIR}/GitUtils.cmake")

# 获取 Git 信息
get_full_git_info(GIT_COMMIT_DEPTH GIT_COMMIT_HASH GIT_CURRENT_BRANCH)
if(GIT_COMMIT_DEPTH)
    set(VERSION_PATCH ${GIT_COMMIT_DEPTH})
endif()

string(TIMESTAMP BUILD_TIME "%Y-%m-%dT%H:%M:%SZ" UTC)
file(WRITE "${OUTPUT_FILE}" "{\n")
file(APPEND "${OUTPUT_FILE}" "  \"PROJECT_VERSION_MAJOR\": \"${VERSION_MAJOR}\",\n")
file(APPEND "${OUTPUT_FILE}" "  \"PROJECT_VERSION_MINOR\": \"${VERSION_MINOR}\",\n")
file(APPEND "${OUTPUT_FILE}" "  \"PROJECT_VERSION_PATCH\": \"${VERSION_PATCH}\",\n")
file(APPEND "${OUTPUT_FILE}" "  \"GIT_COMMIT_HASH\": \"${GIT_COMMIT_HASH}\",\n")
file(APPEND "${OUTPUT_FILE}" "  \"GIT_CURRENT_BRANCH\": \"${GIT_CURRENT_BRANCH}\",\n")
file(APPEND "${OUTPUT_FILE}" "  \"PROJECT_COMPILE_TIME\": \"${BUILD_TIME}\"\n")
file(APPEND "${OUTPUT_FILE}" "}\n")
message(STATUS "[GenerateAppVersionMeta] Generated ${OUTPUT_FILE}")
