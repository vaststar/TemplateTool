find_package(Qt6 REQUIRED COMPONENTS Core Quick Gui Widgets LinguistTools)

qt_standard_project_setup(REQUIRES 6.5)

add_subdirectory(qt)
add_subdirectory(AppRunner)
add_subdirectory(main)

#qt_add_translations(mainEntry
#        INCLUDE_DIRECTORIES
#            ${CMAKE_CURRENT_SOURCE_DIR}/qt/UIView
#        TS_FILES
#            translation_ru.ts)
#
#add_dependencies(mainEntry update_translations)
