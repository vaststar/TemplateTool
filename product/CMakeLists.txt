set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeScripts/BuildModule" "${CMAKE_CURRENT_SOURCE_DIR}/CMakeScripts/FindModule") 

add_subdirectory(framework)
add_subdirectory(commonhead)
add_subdirectory(application)