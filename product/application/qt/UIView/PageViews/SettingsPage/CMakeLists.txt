list(APPEND MODULE_SOURCE_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SettingsPage/SettingsPageController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SettingsPage/SettingsTreeModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SettingsPage/AppearanceSettingsController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SettingsPage/LanguageSettingsController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SettingsPageController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SettingsTreeModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AppearanceSettingsController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LanguageSettingsController.cpp
)
list(APPEND MODULE_QML_TARGET_FILES
    PageViews/SettingsPage/qml/SettingsPage.qml
    PageViews/SettingsPage/qml/SettingsNavPanel.qml
    PageViews/SettingsPage/qml/AppearanceSettingsPanel.qml
    PageViews/SettingsPage/qml/LanguageSettingsPanel.qml
)
set(MODULE_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE} PARENT_SCOPE)
set(MODULE_QML_TARGET_FILES ${MODULE_QML_TARGET_FILES} PARENT_SCOPE)

list(APPEND MODULE_PRIVATE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/SettingsPage)
set(MODULE_PRIVATE_INCLUDE_DIRS ${MODULE_PRIVATE_INCLUDE_DIRS} PARENT_SCOPE)
