include(BuildModule)

set(MODULE_MODULE_NAME SystemUtils)
set(MODULE_IDE_FOLDER framework/utilities/foundation)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Utilities/SystemUtils/SystemUtilsExport.h
    include/ucf/Utilities/SystemUtils/SystemUtils.h
    include/ucf/Utilities/SystemUtils/SystemProxyUtils.h
    include/ucf/Utilities/SystemUtils/CertStoreUtils.h
)

set(MODULE_SOURCE_PRIVATE
    src/SystemUtils.cpp
    src/SystemProxyUtils.cpp
    src/CertStoreUtils.cpp
)

if (WIN32)
    list(APPEND MODULE_SOURCE_PRIVATE
        src/SystemUtils_Win.h
        src/SystemUtils_Win.cpp
        src/SystemProxyUtils_Win.h
        src/SystemProxyUtils_Win.cpp
        src/CertStoreUtils_Win.h
        src/CertStoreUtils_Win.cpp)
elseif (APPLE)
    list(APPEND MODULE_SOURCE_PRIVATE
        src/SystemUtils_Mac.h
        src/SystemUtils_Mac.mm
        src/SystemProxyUtils_Mac.h
        src/SystemProxyUtils_Mac.cpp
        src/CertStoreUtils_Mac.h
        src/CertStoreUtils_Mac.cpp)
elseif (UNIX)
    list(APPEND MODULE_SOURCE_PRIVATE
        src/SystemUtils_Linux.h
        src/SystemUtils_Linux.cpp
        src/SystemProxyUtils_Linux.h
        src/SystemProxyUtils_Linux.cpp
        src/CertStoreUtils_Linux.h
        src/CertStoreUtils_Linux.cpp)
endif()

set(MODULE_DEPENDENCIES_PUBLIC
)

set(MODULE_DEPENDENCIES_PRIVATE
)

if (WIN32)
    list(APPEND MODULE_DEPENDENCIES_PRIVATE crypt32 wininet)
elseif (APPLE)
    list(APPEND MODULE_DEPENDENCIES_PRIVATE "-framework AppKit")
endif()

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

set(MODULE_PUBLIC_DEFINITIONS)
set(MODULE_PRIVATE_DEFINITIONS)
if (UTILITY_LIB_TYPE STREQUAL "STATIC_LIB")
    set(MODULE_PUBLIC_DEFINITIONS SYSTEM_UTILS_STATIC)
else()
    set(MODULE_PRIVATE_DEFINITIONS SYSTEM_UTILS_SHARED)
endif()

BuildModule(
${UTILITY_LIB_TYPE}
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE  ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_PUBLIC_DEFINITIONS ${MODULE_PUBLIC_DEFINITIONS}
TARGET_PRIVATE_DEFINITIONS ${MODULE_PRIVATE_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)
