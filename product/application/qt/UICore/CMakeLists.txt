set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC
    include/UICore/CoreApplication.h
    include/UICore/UICoreExport.h
    include/UICore/CoreQmlApplicationEngine.h
    include/UICore/CoreContext.h
    include/UICore/CoreViewFactory.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    # include/UICore/LoggerDefine.h 
    src/CoreApplication.cpp
    src/CoreQmlApplicationEngine.cpp
    src/CoreContext.cpp
    src/CoreViewFactory.cpp
)


include(BuildQtModule)
BuildQtModule(
    MODULE_NAME 
        UICore
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        Qt6::Gui Qt6::Quick Qt6::Widgets
    TARGET_DEPENDENICES_PUBLIC
        MasterLog 
    TARGET_DEFINITIONS
        UICore_DLL
        UICore_LIB
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    IDE_FOLDER
        UI
)