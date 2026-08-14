include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    ClientInfoViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        ClientInfoViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        ClientInfoViewModelImpl
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/test
)
