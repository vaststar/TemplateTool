include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    MediaCameraViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        MediaCameraViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        MediaCameraViewModelImpl
        CommonHeadFramework
        CoreFrameworkImpl
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/test
)
