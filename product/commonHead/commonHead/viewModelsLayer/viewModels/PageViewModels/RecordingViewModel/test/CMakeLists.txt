include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    RecordingViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        RecordingViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        RecordingViewModelImpl
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewModelsLayer/unit-test
)
