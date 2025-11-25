set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Gui Widgets LinguistTools Multimedia)

qt_standard_project_setup(REQUIRES 6.5)

add_subdirectory(UIAuxiliary)
add_subdirectory(UIComponent)
add_subdirectory(UIView)
add_subdirectory(mainUI)

set_target_properties(all_qmllint PROPERTIES FOLDER UI)

file(GLOB_RECURSE ALL_TRANSLATION_SOURCE_FILES "*.cpp" "*.qml")
set(GENERATED_CPP_FILES ${UCF_CODE_GENERATOR_DIR}/UIResourceStringLoader/ResourceStringMap.cpp)
list(APPEND ALL_TRANSLATION_SOURCE_FILES ${GENERATED_CPP_FILES})

set(Translation_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../resources/translations)
set(Translation_LIB UIView)
set(Translation_TS_FILES        
    ${Translation_DIR}/app_translations_en.ts
    ${Translation_DIR}/app_translations_zh-CN.ts
)

qt_add_translations(
    ${Translation_LIB}  
    TS_FILE_BASE app_translations
    TS_FILE_DIR ${Translation_DIR}
    TS_FILES ${Translation_TS_FILES}
    SOURCES ${ALL_TRANSLATION_SOURCE_FILES}
)

SetIDEFolder(
    TARGET_NAMES
        ${Translation_LIB}_lupdate
        ${Translation_LIB}_lrelease
        ${Translation_LIB}_${Translation_LIB}_translations
        ${Translation_LIB}_resources_3
        update_translations
        release_translations
    FOLDER_NAME
        UI/translations
)

if (TARGET ${Translation_LIB}_lupdate)
    add_dependencies(${Translation_LIB}_lupdate UIResourceStringLoader)
    if (TARGET ${Translation_LIB}_lrelease)
        add_dependencies(${Translation_LIB}_lrelease ${Translation_LIB}_lupdate)
    endif()
endif()

if (TARGET release_translations AND TARGET update_translations)
    add_dependencies(release_translations update_translations)
endif()

if (TARGET release_translations)
    add_dependencies(${Translation_LIB} release_translations )
endif()
