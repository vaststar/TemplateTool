include(BuildInterface)

BuildInterface(
    MODULE_NAME 
        VMNotificationHelper
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        include/comomonHead/commonHeadUtils/VMNotificationHelper/VMNotificationHelper.h
        include/comomonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
)