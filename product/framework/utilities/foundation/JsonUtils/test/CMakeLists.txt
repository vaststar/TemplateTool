include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    JsonValue_tests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        JsonUtilsTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        JsonUtils
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/utilities/test
)
