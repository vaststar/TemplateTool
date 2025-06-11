cmake_minimum_required(VERSION 3.23)

set(ProjectName "ToolFramework")
set(ProjectDesc "ToolFramework for app")

project(${ProjectName} DESCRIPTION ${ProjectDesc}
                       LANGUAGES CXX
        )

add_subdirectory(thirdparty)
add_subdirectory(utilities)
add_subdirectory(services)