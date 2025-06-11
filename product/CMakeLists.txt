set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeScripts/BuildModule" "${CMAKE_CURRENT_SOURCE_DIR}/CMakeScripts/FindModule" "${CMAKE_CURRENT_SOURCE_DIR}/CMakeScripts/UtilModule")
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

add_subdirectory(framework)
add_subdirectory(commonHead)
add_subdirectory(application)