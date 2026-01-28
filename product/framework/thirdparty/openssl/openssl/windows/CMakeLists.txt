# =============================================================================
# OpenSSL - Windows Platform
# =============================================================================

include(DetectPlatform)
detect_platform(PLATFORM_TYPE)

# Determine architecture directory
if(PLATFORM_TYPE STREQUAL "WINDOWS_X64")
    set(SSL_ARCH_DIR "x64")
elseif(PLATFORM_TYPE STREQUAL "WINDOWS_X86")
    set(SSL_ARCH_DIR "x86")
elseif(PLATFORM_TYPE STREQUAL "WINDOWS_ARM64")
    set(SSL_ARCH_DIR "arm64")
else()
    message(FATAL_ERROR "[OpenSSL] Unsupported Windows architecture: ${PLATFORM_TYPE}")
endif()

set(SSL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${SSL_ARCH_DIR}")

# Find libraries
find_library(SSL_CRYPTO_LIB
    NAMES libcrypto
    PATHS "${SSL_ROOT}/lib"
    NO_DEFAULT_PATH
)

find_library(SSL_SSL_LIB
    NAMES libssl
    PATHS "${SSL_ROOT}/lib"
    NO_DEFAULT_PATH
)

if(NOT SSL_CRYPTO_LIB OR NOT SSL_SSL_LIB)
    message(FATAL_ERROR "[OpenSSL] Libraries not found in ${SSL_ROOT}/lib")
endif()

# Find DLLs
file(GLOB SSL_DLLS "${SSL_ROOT}/bin/*.dll")
if(NOT SSL_DLLS)
    message(FATAL_ERROR "[OpenSSL] DLLs not found in ${SSL_ROOT}/bin")
endif()

# Find include directory
set(SSL_INCLUDE "${SSL_ROOT}/include")
if(NOT EXISTS "${SSL_INCLUDE}/openssl/ssl.h")
    message(FATAL_ERROR "[OpenSSL] Headers not found in ${SSL_INCLUDE}")
endif()

message(STATUS "[OpenSSL] Windows ${SSL_ARCH_DIR}")
message(STATUS "[OpenSSL] Crypto: ${SSL_CRYPTO_LIB}")
message(STATUS "[OpenSSL] SSL: ${SSL_SSL_LIB}")
message(STATUS "[OpenSSL] DLLs: ${SSL_DLLS}")
message(STATUS "[OpenSSL] Include: ${SSL_INCLUDE}")

# Create copy target
add_custom_target(copy_ssl)
set_target_properties(copy_ssl PROPERTIES FOLDER framework/thirdparty/openssl)

add_custom_command(TARGET copy_ssl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SSL_DLLS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/"
    COMMAND_EXPAND_LISTS
    COMMENT "[OpenSSL] Copying DLLs to output directory"
)

# Create interface library
add_library(openssl INTERFACE)
target_include_directories(openssl INTERFACE "${SSL_INCLUDE}")
target_link_libraries(openssl INTERFACE "${SSL_CRYPTO_LIB}" "${SSL_SSL_LIB}")
add_dependencies(openssl copy_ssl)

# Install DLLs to installation directory
install(FILES ${SSL_DLLS} DESTINATION bin)
