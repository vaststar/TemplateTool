message(STATUS "CMake version: ${CMAKE_VERSION}")

set(CMAKE_MODULE_PATH 
    ${CMAKE_MODULE_PATH} 
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeScripts/BuildModule" 
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeScripts/FindModule" 
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeScripts/UtilModule"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeScripts/VersionModule"
)

set(UCF_CODE_GENERATOR_DIR ${CMAKE_BINARY_DIR}/codegen)
message(STATUS "set UCF_CODE_GENERATOR_DIR: ${UCF_CODE_GENERATOR_DIR}")

add_subdirectory(framework)
add_subdirectory(commonHead)
add_subdirectory(application)