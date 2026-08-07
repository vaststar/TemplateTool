add_subdirectory(fakes)
include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    ImageServiceTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        ImageServiceTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        ImageServiceImpl
        CoreFramework
        FakeCoreFramework
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
        ${CMAKE_CURRENT_SOURCE_DIR}/../ImageServiceImpl/src
    IDE_FOLDER
        framework/Services/unit-test
)
