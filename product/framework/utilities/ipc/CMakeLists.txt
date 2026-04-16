if (WIN32)
    set(ADD_UTILITIES_DEFINITIONS Utilities_DLL)
else()
    set(ADD_UTILITIES_DEFINITIONS)
endif()

add_subdirectory(NotificationHelper)
add_subdirectory(TcpChannelUtils)
add_subdirectory(ProcessBridgeUtils)
