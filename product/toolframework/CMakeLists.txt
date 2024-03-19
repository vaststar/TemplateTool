cmake_minimum_required(VERSION 3.20)

set(ProjectName "ToolFramework")
set(ProjectDesc "ToolFramework for app")

project(${ProjectName} DESCRIPTION ${ProjectDesc}
                       LANGUAGES CXX
        )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeScripts") 

add_subdirectory(thirdparty)
add_subdirectory(Utilities)
add_subdirectory(Services)
add_subdirectory(unit-test)