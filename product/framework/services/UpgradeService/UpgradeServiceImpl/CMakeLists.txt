include(BuildModule)

# UpgradeServiceImpl (SHARED) - concrete implementation with FSM + sub-managers.
set(MODULE_MODULE_NAME UpgradeServiceImpl)
set(MODULE_IDE_FOLDER framework/Services)
set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Services/UpgradeService/UpgradeServiceImplExport.h
    include/ucf/Services/UpgradeService/UpgradeServiceCreator.h
)

set(MODULE_SOURCE_PRIVATE
    src/UpgradeService.h
    src/UpgradeService.cpp
    src/UpgradeServiceLogger.h
    # Manager layer
    src/UpgradeManager.h
    src/UpgradeManager.cpp
    # FSM
    src/fsm/UpgradeContext.h
    src/fsm/UpgradeEvents.h
    src/fsm/UpgradeStates.h
    src/fsm/UpgradeTransitions.inl
    # Sub-managers
    src/check/UpgradeCheckManager.h
    src/check/UpgradeCheckManager.cpp
    src/download/UpgradeDownloadManager.h
    src/download/UpgradeDownloadManager.cpp
    src/install/UpgradeInstallManager.h
    src/install/UpgradeInstallManager.cpp
)

set(MODULE_DEPENDENCIES_PUBLIC
    UpgradeServiceApi
)
set(MODULE_DEPENDENCIES_PRIVATE
    CoreFrameworkApi
    NetworkServiceApi
    ClientInfoServiceApi
    ArchiveUtils
    FilePathUtils
    JsonUtils
    VersionUtils
    OSUtils
    LoggerUtils
    CompressionUtils
    StateMachineUtils
    NotificationHelper
    ProcessBridgeUtils
    openssl
)
set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(MODULE_PUBLIC_DEFINITIONS)
set(MODULE_PRIVATE_DEFINITIONS)
if (SERVICE_LIB_TYPE STREQUAL "STATIC_LIB")
    # Static lib: macro is empty but must propagate to consumers -> PUBLIC
    set(MODULE_PUBLIC_DEFINITIONS UPGRADE_SERVICE_IMPL_STATIC)
else()
    # Shared lib: only this module exports; consumers use dllimport -> PRIVATE
    set(MODULE_PRIVATE_DEFINITIONS UPGRADE_SERVICE_IMPL_SHARED)
endif()

BuildModule(
    ${SERVICE_LIB_TYPE}
    MODULE_NAME ${MODULE_MODULE_NAME}
    TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
    TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
    TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
    TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
    TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
    TARGET_PUBLIC_DEFINITIONS ${MODULE_PUBLIC_DEFINITIONS}
    TARGET_PRIVATE_DEFINITIONS ${MODULE_PRIVATE_DEFINITIONS}
    IDE_FOLDER ${MODULE_IDE_FOLDER}
)

