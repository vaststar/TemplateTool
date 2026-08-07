add_subdirectory(fakes)
include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    CameraDirectoryServiceTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        CameraDirectoryServiceTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        CameraDirectoryServiceImpl
        CoreFramework
        DataWarehouseServiceApi
        FakeCoreFramework
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
        ${CMAKE_CURRENT_SOURCE_DIR}/../CameraDirectoryServiceImpl/src
    IDE_FOLDER
        framework/Services/unit-test
)
