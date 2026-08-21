set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeScripts/BuildModule"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeScripts/UtilModule"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeScripts/VersionModule"
    "${CMAKE_CURRENT_SOURCE_DIR}/framework/thirdparty/Catch2/Catch2/extras"
)

set(UCF_CODE_GENERATOR_DIR ${CMAKE_BINARY_DIR}/codegen)
message(STATUS
    "[TemplateTool] Code generator: ${UCF_CODE_GENERATOR_DIR}")

# Runtime payload producers register against this project-scoped directory.
# Consumers read the registry after framework configuration has completed.
set(TT_RUNTIME_PAYLOAD_REGISTRY_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(framework)
add_subdirectory(commonhead)
add_subdirectory(apprunner)
add_subdirectory(application)
add_subdirectory(packaging)
