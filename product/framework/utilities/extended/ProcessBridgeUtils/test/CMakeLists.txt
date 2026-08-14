include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    ProcessBridgeTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        ProcessBridgeTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        ProcessBridgeUtils
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/utilities/test
)
