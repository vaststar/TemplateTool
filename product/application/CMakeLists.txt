find_package(Qt6 REQUIRED COMPONENTS Quick Gui Widgets)

qt_standard_project_setup(REQUIRES 6.5)

add_subdirectory(qt)
add_subdirectory(AppRunner)
add_subdirectory(main)