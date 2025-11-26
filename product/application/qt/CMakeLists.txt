set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Gui Widgets LinguistTools Multimedia)

qt_standard_project_setup(REQUIRES 6.5)

add_subdirectory(UIAuxiliary)
add_subdirectory(UIComponent)
add_subdirectory(UITranslation)
add_subdirectory(UIView)
add_subdirectory(mainUI)

set_target_properties(all_qmllint PROPERTIES FOLDER UI)
