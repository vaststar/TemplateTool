set(ProjectName "CommonHead")
set(ProjectDesc "CommonHead for app")

project(${ProjectName} DESCRIPTION ${ProjectDesc}
                       LANGUAGES CXX
        )

add_subdirectory(utilities)
add_subdirectory(infrastructure)
add_subdirectory(CommonHeadFramework)
add_subdirectory(viewmodels)
