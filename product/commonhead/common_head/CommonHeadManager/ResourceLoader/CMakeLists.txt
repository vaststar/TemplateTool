include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/commonHead/ResourceLoader/IResourceLoader.h
    include/commonHead/ResourceLoader/ResourceLoaderModel.h
    include/commonHead/ResourceLoader/ResourceLocalizedStringModel.h
    include/commonHead/ResourceLoader/IResourceStringLoader.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/ResourceLoader.h
    src/ResourceLoader.cpp
    src/FontSet.h
    src/FontSet.cpp
    src/ColorSet.h
    src/ColorSet.cpp
    src/ResourceTheme.h
    src/ResourceTheme.cpp
    src/ColorConstant.h
    src/IResourceStringLoader.cpp
    src/ResourceThemeLoader.h
    src/ResourceThemeLoader.cpp
    src/ColorBuilder.h
    src/ColorBuilder.cpp
)

BuildModule(
    MODULE_NAME 
        ResourceLoader
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PUBLIC
        CommonHeadCommonFile
    TARGET_DEPENDENICES_PRIVATE
        ClientInfoService
        CoreFramework
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        ${ADD_COMMONHEAD_DEFINITIONS}
    IDE_FOLDER
        commonhead
)