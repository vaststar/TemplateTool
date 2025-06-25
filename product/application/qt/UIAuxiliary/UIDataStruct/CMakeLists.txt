set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC
    include/UIDataStruct/UIDataStructExport.h
    include/UIDataStruct/UIElementData.h
    include/UIDataStruct/UIDataUtils.h
    include/UIDataStruct/UIStrings.h
    include/UIDataStruct/UILanguage.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/LoggerDefine.h
    src/UIElementData.cpp
    src/UIDataUtils.cpp
    src/UIStrings.cpp
    src/UILanguage.cpp
)

if (WIN32)
    set(ADD_DEFINITIONS UIDataStruct_DLL) #UIDataStruct_DLL UIDataStruct_LIB
else()
    set(ADD_DEFINITIONS)
endif()


# set(UIDataStruct_QML_PUBLIC_BUILD_INTERFACE_FOLDER
#     ${CMAKE_CURRENT_SOURCE_DIR}/include/UIDataStruct
    
# )

# set(UIDataStruct_QML_TARGET_SOURCES
#     include/UIDataStruct/UIColorSet.h
#     include/UIDataStruct/UIFontSet.h
#     src/UIColorSet.cpp
#     src/UIFontSet.cpp
# )

include(BuildQtModule)
BuildQtModule(
    MODULE_NAME 
        UIDataStruct
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        ViewModels
        ResourceLoader
        MasterLog
        UICore
    TARGET_DEPENDENICES_PUBLIC
        Qt6::Widgets
        Qt6::Qml
    TARGET_DEFINITIONS
        ${ADD_DEFINITIONS}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    IDE_FOLDER
        UI
    # QML_TARGET_URI
    #     UIDataStruct
    # QML_TARGET_SOURCES
    #     ${UIDataStruct_QML_TARGET_SOURCES}
    # QML_PUBLIC_BUILD_INTERFACE_FOLDER
    #     ${UIDataStruct_QML_PUBLIC_BUILD_INTERFACE_FOLDER}
)