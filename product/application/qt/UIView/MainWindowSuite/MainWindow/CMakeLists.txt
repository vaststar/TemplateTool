list(APPEND MODULE_SOURCE_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MainWindowController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MainWindowContentPageRegistry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MainWindowController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MainWindowContentPageRegistry.cpp
)
list(APPEND MODULE_QML_TARGET_FILES
    MainWindowSuite/MainWindow/qml/MainWindow.qml
    MainWindowSuite/MainWindow/qml/MainWindowContent.qml
    MainWindowSuite/MainWindow/qml/WindowResizeHandler.qml
)
set(MODULE_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE} PARENT_SCOPE)
set(MODULE_QML_TARGET_FILES ${MODULE_QML_TARGET_FILES} PARENT_SCOPE)

list(APPEND MODULE_PRIVATE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(MODULE_PRIVATE_INCLUDE_DIRS ${MODULE_PRIVATE_INCLUDE_DIRS} PARENT_SCOPE)
