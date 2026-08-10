add_subdirectory(fakes)
include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    MiniAppServiceTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        MiniAppServiceTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        MiniAppServiceImpl
        CoreFrameworkApi
        FakeCoreFramework
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/Services/unit-test
)
