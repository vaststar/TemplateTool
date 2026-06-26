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
        NetworkService
        FakeNetworkService
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/Services/unit-test
)
