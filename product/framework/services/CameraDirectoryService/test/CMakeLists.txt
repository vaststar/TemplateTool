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
        CoreFrameworkApi
        DataWarehouseServiceApi
        FakeCoreFramework
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/services/test
)
