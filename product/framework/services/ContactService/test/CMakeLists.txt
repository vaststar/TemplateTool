add_subdirectory(fakes)
include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    ContactServiceTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        ContactServiceTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        ContactService
        FakeContactService
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/Services/unit-test
)
