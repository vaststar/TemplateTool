include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    StabilityServiceTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        StabilityServiceTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        StabilityServiceImpl
        ClientInfoServiceApi
        CoreFrameworkApi
        FakeCoreFramework
        FakeClientInfoService
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/services/test
)
