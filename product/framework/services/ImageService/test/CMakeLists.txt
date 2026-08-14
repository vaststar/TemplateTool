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
        CoreFrameworkApi
        FakeCoreFramework
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/services/test
)
