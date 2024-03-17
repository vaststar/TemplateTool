cmake_minimum_required(VERSION 3.16)

project(tool_windows VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 6.2 COMPONENTS Quick Gui REQUIRED)

qt_standard_project_setup(REQUIRES 6.5)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
qt_add_executable(${PROJECT_NAME}
    main.cpp
    # cpp/Datas.cpp
    # datas.h
)

if(${CMAKE_CXX_COMPILER} MATCHES ".*/(g\\+\\+)(\\..*)")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "-mwindows")
elseif(${CMAKE_CXX_COMPILER} MATCHES ".*/(cl)(\\..*)")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()



qt_add_qml_module(${PROJECT_NAME}
    URI mainqml
    VERSION 1.0
    QML_FILES
        main.qml
)

add_subdirectory(mylib)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Gui Qt6::Quick MyQMLplugin MasterLog CoreFramework)