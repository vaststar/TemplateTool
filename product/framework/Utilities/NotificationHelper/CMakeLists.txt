include(BuildInterface)

BuildInterface(
    MODULE_NAME 
        NotificationHelper
    PUBLIC_BUILD_INTERFACE_FOLDER
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC_INSTALL_INTERFACE_FOLDER
        include
    INSTALL_PUBLIC_HEADER 
        include/NotificationHelper/NotificationHelperExport.h
        include/NotificationHelper/NotificationHelper.h
        include/NotificationHelper/INotificationHelper.h
        include/NotificationHelper/NotificationHelperImpl.h
    IDE_FOLDER
        framework/Utilities
)


# include(BuildModule)

# BuildModule(
#     MODULE_NAME 
#     NotificationHelper
#     TARGET_SOURCE 
#         include/NotificationHelper/NotificationHelperExport.h
#         include/NotificationHelper/NotificationHelper.h
#         # include/NotificationHelper/IHelper.h
#         # src/IHelper.cpp
#     PUBLIC_BUILD_INTERFACE_FOLDER
#         ${CMAKE_CURRENT_SOURCE_DIR}/include
#     PUBLIC_INSTALL_INTERFACE_FOLDER
#         include
#     # PRIVATE_BUILD_FOLDER
#         # src
#     TARGET_DEFINITIONS
#         ${ADD_UTILITIES_DEFINITIONS}
#     INSTALL_PUBLIC_HEADER 
#         include/NotificationHelper/NotificationHelper.h 
#         include/NotificationHelper/NotificationHelperExport.h
#         # include/NotificationHelper/IHelper.h 
#     IDE_FOLDER
#         Utilities
# )
