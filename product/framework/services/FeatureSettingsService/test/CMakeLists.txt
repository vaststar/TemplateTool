add_subdirectory(fakes)
include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    FeatureSettingsServiceTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        FeatureSettingsServiceTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        FeatureSettingsServiceImpl
        CoreFramework
        DataWarehouseServiceApi
        FakeCoreFramework
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
        ${CMAKE_CURRENT_SOURCE_DIR}/../FeatureSettingsServiceImpl/src
    IDE_FOLDER
        framework/Services/unit-test
)
