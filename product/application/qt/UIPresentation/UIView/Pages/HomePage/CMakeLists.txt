UIViewAppendBuildInputs(
    SOURCES
        include/HomePage/HomePageController.h
        src/HomePageController.cpp
    QML_ALIAS_PREFIX PageViews/HomePage
    QML_FILES
        qml/HomePage.qml
    PRIVATE_INCLUDE_DIRS
        include
        include/HomePage
)
