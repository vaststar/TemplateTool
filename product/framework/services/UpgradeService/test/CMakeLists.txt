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
        CoreFrameworkApi
        FakeCoreFramework
        NetworkServiceApi
        ClientInfoServiceApi
        StateMachineUtils
        FilePathUtils
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/services/test
)
