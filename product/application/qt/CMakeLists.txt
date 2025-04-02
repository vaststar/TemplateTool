set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Gui Widgets LinguistTools Multimedia)

qt_standard_project_setup(REQUIRES 6.5)

add_subdirectory(UIAuxiliary)
add_subdirectory(AppContext)
add_subdirectory(UIView)
add_subdirectory(mainUI)

set_target_properties(all_qmllint PROPERTIES FOLDER UI)


file(GLOB_RECURSE ALL_TRANSLATION_SOURCE_FILES "*.cpp" "*.qml")
set(Translation_DIR ../resources/translations)
set(Translation_LIB UIView)
#set(ALL_TRANSLATION_SOURCE_FILES ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC} ${TARGET_SOURCE_FILES_PRIVATE} ${UIVIEW_QML_TARGET_FILES})

qt_add_translations(${Translation_LIB}  
    #TS_FILE_BASE UIVIEW_translations_en
    #TS_FILE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../resources/translations
    TS_FILES 
        ${Translation_DIR}/app_translations_en.ts
        ${Translation_DIR}/app_translations_zh-CN.ts
    SOURCES 
        ${ALL_TRANSLATION_SOURCE_FILES}
)

SetIDEFolder(
    TARGET_NAMES
        ${Translation_LIB}_lupdate
        ${Translation_LIB}_lrelease
        ${Translation_LIB}_${Translation_LIB}_translations
        update_translations
        release_translations
    FOLDER_NAME
        UI/translations
)

add_dependencies(${Translation_LIB} update_translations)