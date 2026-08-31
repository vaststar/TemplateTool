include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    UTMessageDialogPresetTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        UTMessageDialogPresetTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        Qt6::Core
    TARGET_INCLUDE_FOLDER
        ..
    IDE_FOLDER
        UI/UIComponent/test
)
