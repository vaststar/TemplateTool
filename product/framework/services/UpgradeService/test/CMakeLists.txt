cmake_minimum_required(VERSION 3.11)

include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    UpgradeServiceTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        UpgradeServiceTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        UpgradeService
        CoreFramework
        FakeCoreFramework
        NetworkService
        ClientInfoService
        StateMachineUtils
        FilePathUtils
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
    IDE_FOLDER
        framework/Services/unit-test
)
