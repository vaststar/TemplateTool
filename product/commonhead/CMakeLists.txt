cmake_minimum_required(VERSION 3.20)

set(ProjectName "CommonHead")
set(ProjectDesc "CommonHead for app")

project(${ProjectName} DESCRIPTION ${ProjectDesc}
                       LANGUAGES CXX
        )

add_subdirectory(common_head)
add_subdirectory(unit_test)