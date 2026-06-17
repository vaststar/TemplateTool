# ==========================================
# UIToolKitGallery - standalone UIComponent debug executable
# Enable with: -DBUILD_COMPONENT_GALLERY=ON
# ==========================================
project(UIToolKitGallery LANGUAGES CXX)

set(GALLERY_TARGET UIToolKitGallery)

qt_add_executable(${GALLERY_TARGET}
    src/main.cpp
    src/MockResourceLoader.cpp
    src/MockResourceLoader.h
    src/MockCommonHeadFramework.h
    src/GalleryThemeController.h
)

set_target_properties(${GALLERY_TARGET} PROPERTIES
    AUTOMOC ON
)

qt_add_qml_module(${GALLERY_TARGET}
    URI UIToolKitGallery
    VERSION 1.0
    QML_FILES
        qml/Gallery.qml
)

target_compile_features(${GALLERY_TARGET} PRIVATE cxx_std_20)

target_link_libraries(${GALLERY_TARGET} PRIVATE
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::Widgets
    # UI foundation
    UICore
    UIResourceLoaderManager
    # commonHead: real color/font/asset loaders + framework interface
    ResourceLoader
    CommonHeadFramework
    # The components under test
    UIComponentBase
    UTComponent
    UTComposite
)

# Keep a console window so logs are visible while debugging components.
if(WIN32)
    set_target_properties(${GALLERY_TARGET} PROPERTIES WIN32_EXECUTABLE OFF)
endif()

set_target_properties(${GALLERY_TARGET} PROPERTIES
    FOLDER UI/UIComponent/UIToolKitGallery
)
