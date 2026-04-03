include(BuildModule)

set(MODULE_MODULE_NAME UpgradeService)
set(MODULE_IDE_FOLDER framework/Services)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Services/UpgradeService/IUpgradeService.h
    include/ucf/Services/UpgradeService/IUpgradeServiceCallback.h
    include/ucf/Services/UpgradeService/UpgradeModel.h
)

set(MODULE_SOURCE_PRIVATE
    src/UpgradeService.h
    src/UpgradeService.cpp
    src/UpgradeServiceLogger.h
    # Manager layer
    src/UpgradeManager.h
    src/UpgradeManager.cpp
    # FSM
    src/fsm/UpgradeEvents.h
    src/fsm/UpgradeStates.h
    # Sub-managers
    src/check/UpgradeCheckManager.h
    src/check/UpgradeCheckManager.cpp
    src/download/UpgradeDownloadManager.h
    src/download/UpgradeDownloadManager.cpp
    src/install/UpgradeInstallManager.h
    src/install/UpgradeInstallManager.cpp
)

set(MODULE_DEPENDENCIES_PUBLIC
    ServiceDeclaration
)

set(MODULE_DEPENDENCIES_PRIVATE
    CoreFramework
    NetworkService
    ClientInfoService
    ArchiveUtils
    FilePathUtils
    JsonUtils
    LoggerUtils
    CompressionUtils
    StateMachineUtils
    NotificationHelper
    openssl
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

BuildModule(
    MODULE_NAME ${MODULE_MODULE_NAME}
    TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
    TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
    TARGET_DEFINITIONS ${ADD_SERVICE_DEFINITIONS}
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)

add_subdirectory(test)
