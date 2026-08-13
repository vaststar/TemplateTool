UIViewAppendBuildInputs(
    SOURCES
        include/AboutPage/AboutPageController.h
        src/AboutPageController.cpp
    QML_ALIAS_PREFIX PageViews/AboutPage
    QML_FILES
        qml/AboutDialog.qml
    PRIVATE_INCLUDE_DIRS
        include
        include/AboutPage
    PRIVATE_DEPENDENCIES
        ClientInfoViewModelApi
)
