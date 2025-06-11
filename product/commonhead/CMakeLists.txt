cmake_minimum_required(VERSION 3.20)

set(ProjectName "CommonHead")
set(ProjectDesc "CommonHead for app")

project(${ProjectName} DESCRIPTION ${ProjectDesc}
                       LANGUAGES CXX
        )

add_subdirectory(commonHead)
add_subdirectory(unit_test)