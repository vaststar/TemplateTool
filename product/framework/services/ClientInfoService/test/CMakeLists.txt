add_subdirectory(fakes)
include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    ClientInfoServiceTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        ClientInfoServiceTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        DataWarehouseServiceApi
        ClientInfoServiceImpl
        CoreFrameworkApi
        FakeCoreFramework
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/services/test
)
