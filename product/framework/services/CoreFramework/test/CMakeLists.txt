add_subdirectory(fakes)
include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    CoreFrameworkTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        CoreFrameworkTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        CoreFrameworkImpl
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/services/test
)
