list(APPEND MODULE_SOURCE_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ContactListItemModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ContactsPageController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ContactListItemModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ContactsPageController.cpp
)
list(APPEND MODULE_QML_TARGET_FILES
    PageViews/ContactsPage/qml/ContactsPage.qml
    PageViews/ContactsPage/qml/ContactListPanel.qml
    PageViews/ContactsPage/qml/ContactDetailPanel.qml
)
set(MODULE_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE} PARENT_SCOPE)
set(MODULE_QML_TARGET_FILES ${MODULE_QML_TARGET_FILES} PARENT_SCOPE)

list(APPEND MODULE_PRIVATE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(MODULE_PRIVATE_INCLUDE_DIRS ${MODULE_PRIVATE_INCLUDE_DIRS} PARENT_SCOPE)
