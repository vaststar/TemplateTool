include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    PerformanceServiceTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        PerformanceServiceTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        PerformanceServiceImpl
        CoreFrameworkApi
        FakeCoreFramework
        SinkNotifier
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/Services/unit-test
)
