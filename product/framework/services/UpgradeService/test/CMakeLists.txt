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
        UpgradeServiceImpl
        CoreFramework
        FakeCoreFramework
        NetworkServiceApi
        ClientInfoServiceApi
        StateMachineUtils
        FilePathUtils
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
        ${CMAKE_CURRENT_SOURCE_DIR}/../UpgradeServiceImpl/src
    IDE_FOLDER
        framework/Services/unit-test
)
