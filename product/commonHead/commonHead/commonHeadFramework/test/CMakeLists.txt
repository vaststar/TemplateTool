include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    CommonHeadFrameworkTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        CommonHeadFrameworkTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        CommonHeadFramework
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/unit-test
)
