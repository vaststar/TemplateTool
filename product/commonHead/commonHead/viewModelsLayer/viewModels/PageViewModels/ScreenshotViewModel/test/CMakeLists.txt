include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    ScreenshotViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        ScreenshotViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        ScreenshotViewModelImpl
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewModelsLayer/unit-test
)
