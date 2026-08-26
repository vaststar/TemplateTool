add_subdirectory(fakes)
include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    MediaServiceTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        MediaServiceTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        MediaServiceImpl
        CoreFrameworkApi
        FakeCoreFramework
    TARGET_INCLUDE_FOLDER
        .
        ${CMAKE_CURRENT_SOURCE_DIR}/../MediaServiceImpl/src
    IDE_FOLDER
        framework/services/test
)
