set(ProjectName "CommonHead")
set(ProjectDesc "CommonHead for app")

project(${ProjectName} DESCRIPTION ${ProjectDesc}
                       LANGUAGES CXX
        )

option(UCF_BUILD_COMMONHEAD_SHARED
    "Build CommonHead modules as shared libraries (DLL). OFF builds them as static libraries."
    ON
)

if(UCF_BUILD_COMMONHEAD_SHARED)
    set(COMMONHEAD_LIB_TYPE SHARED_LIB)
else()
    set(COMMONHEAD_LIB_TYPE STATIC_LIB)
endif()

add_subdirectory(utilities)
add_subdirectory(infrastructure)
add_subdirectory(CommonHeadFramework)
add_subdirectory(viewmodels)
