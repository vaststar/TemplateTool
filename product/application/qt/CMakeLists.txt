find_package(Qt6 6.5 COMPONENTS Quick Gui REQUIRED)
qt_standard_project_setup(REQUIRES 6.5)

add_subdirectory(main)
add_subdirectory(UIViews)
add_subdirectory(mainUI)