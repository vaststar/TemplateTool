add_subdirectory(fakes)
include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    CoreFrameworkTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        CoreFrameworkTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        Catch2::Catch2WithMain
        CoreFrameworkImpl
        FakeCoreFramework
    TARGET_INCLUDE_FOLDER
        .
        ${CMAKE_CURRENT_SOURCE_DIR}/../CoreFrameworkImpl/src
    IDE_FOLDER
        framework/Services/unit-test
)
