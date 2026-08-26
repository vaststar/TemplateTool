include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    TranslatorManagerTestMain.cpp
    TranslatorManagerTests.cpp
)

BuildUnitTestModule(
    NO_DEFAULT_CATCH_MAIN
    MODULE_NAME
        TranslatorManagerTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        TranslatorManager
        UIResourceTranslation
        UIAppCore
        Catch2::Catch2
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        UI/UIAuxiliary/UIBusiness/test
)
