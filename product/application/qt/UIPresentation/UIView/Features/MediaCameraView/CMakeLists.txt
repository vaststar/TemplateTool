UIViewAppendBuildInputs(
    SOURCES
        include/MediaCameraView/MediaCameraViewController.h
        src/MediaCameraViewController.cpp
    QML_ALIAS_PREFIX MediaCameraView
    QML_FILES
        qml/MediaCameraView.qml
    PRIVATE_INCLUDE_DIRS
        include
        include/MediaCameraView
    PRIVATE_DEPENDENCIES
        MediaCameraViewModelApi
)
