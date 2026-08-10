add_subdirectory(fakes)
include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    NetworkServiceTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        NetworkServiceTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        NetworkServiceImpl
        CoreFrameworkApi
        FakeCoreFramework
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
        ${CMAKE_CURRENT_SOURCE_DIR}/../NetworkServiceImpl/src
    IDE_FOLDER
        framework/Services/unit-test
)
