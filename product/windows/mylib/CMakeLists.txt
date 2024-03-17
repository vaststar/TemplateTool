qt_add_library(MyQML STATIC)

qt_add_qml_module(MyQML
    URI mylib
    VERSION 1.0
    QML_FILES
        qml/FramedImage.qml
    SOURCES
    cpp/Datas.h cpp/Datas.cpp
    RESOURCES
        img/world.png
)

target_include_directories(MyQML PUBLIC 
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cpp>
                                $<INSTALL_INTERFACE:${PROJECT_NAME}/include>
                                PRIVATE ${PROJECT_BINARY_DIR}
)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Gui Qt6::Quick)