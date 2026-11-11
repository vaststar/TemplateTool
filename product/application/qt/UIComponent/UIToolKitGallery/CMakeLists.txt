# ==========================================
# UIToolKitGallery - standalone UIComponent debug executable
# Enable with: -DBUILD_COMPONENT_GALLERY=ON
# ==========================================
include(ApplyProjectTargetOptions)

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

# Register new showcase cases without maintaining a second case list.
file(GLOB GALLERY_CASE_QML_FILES
    CONFIGURE_DEPENDS
    LIST_DIRECTORIES false
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/qml/cases/*Case.qml"
)

qt_add_qml_module(${GALLERY_TARGET}
    URI UIToolKitGallery
    VERSION 1.0
    QML_FILES
        qml/Gallery.qml
        qml/GalleryCatalog.qml
        qml/GalleryNavigation.qml
        qml/common/GalleryCase.qml
        qml/common/GalleryCategory.qml
        qml/common/GalleryDemo.qml
        ${GALLERY_CASE_QML_FILES}
)

ApplyProjectTargetOptions(TARGET ${GALLERY_TARGET})

target_link_libraries(${GALLERY_TARGET} PRIVATE
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::Widgets
    # UI foundation
    UICore
    UIResourceLoaderManager
    # commonHead: real color/font/asset loaders + framework interface
    ResourceLoaderApi
    ResourceColorLoaderImpl
    ResourceFontLoaderImpl
    ResourceAssetLoaderImpl
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
