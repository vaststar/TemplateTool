# =============================================================================
# OpenSSL - macOS Platform
# =============================================================================

include(DetectPlatform)
detect_platform(PLATFORM_TYPE)

# Determine architecture directory
if(PLATFORM_TYPE STREQUAL "MAC_ARM")
    set(SSL_ARCH_DIR "arm")
elseif(PLATFORM_TYPE STREQUAL "MAC_INTEL")
    set(SSL_ARCH_DIR "intel")
else()
    message(FATAL_ERROR "[OpenSSL] Unsupported macOS architecture: ${PLATFORM_TYPE}")
endif()

set(SSL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${SSL_ARCH_DIR}")

# Find libraries
find_library(SSL_CRYPTO_LIB
    NAMES crypto libcrypto.dylib
    PATHS "${SSL_ROOT}/lib"
    NO_DEFAULT_PATH
)

find_library(SSL_SSL_LIB
    NAMES ssl libssl.dylib
    PATHS "${SSL_ROOT}/lib"
    NO_DEFAULT_PATH
)

if(NOT SSL_CRYPTO_LIB OR NOT SSL_SSL_LIB)
    message(FATAL_ERROR "[OpenSSL] Libraries not found in ${SSL_ROOT}/lib")
endif()

# Find dylibs for copying
file(GLOB SSL_DYLIBS "${SSL_ROOT}/lib/*.dylib")

# Find include directory
set(SSL_INCLUDE "${SSL_ROOT}/include")
if(NOT EXISTS "${SSL_INCLUDE}/openssl/ssl.h")
    message(FATAL_ERROR "[OpenSSL] Headers not found in ${SSL_INCLUDE}")
endif()

message(STATUS "[OpenSSL] macOS ${SSL_ARCH_DIR}")
message(STATUS "[OpenSSL] Crypto: ${SSL_CRYPTO_LIB}")
message(STATUS "[OpenSSL] SSL: ${SSL_SSL_LIB}")
message(STATUS "[OpenSSL] Dylibs: ${SSL_DYLIBS}")
message(STATUS "[OpenSSL] Include: ${SSL_INCLUDE}")

# Create copy target for dylibs
if(SSL_DYLIBS)
    add_custom_target(copy_ssl)
    set_target_properties(copy_ssl PROPERTIES FOLDER framework/thirdparty/openssl)

    add_custom_command(TARGET copy_ssl POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SSL_DYLIBS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/"
        COMMAND_EXPAND_LISTS
        COMMENT "[OpenSSL] Copying dylibs to output directory"
    )
endif()

# Create interface library
add_library(openssl INTERFACE)
target_include_directories(openssl INTERFACE "${SSL_INCLUDE}")
target_link_libraries(openssl INTERFACE "${SSL_CRYPTO_LIB}" "${SSL_SSL_LIB}")

if(TARGET copy_ssl)
    add_dependencies(openssl copy_ssl)
endif()

# Install dylibs to installation directory
if(SSL_DYLIBS)
    install(FILES ${SSL_DYLIBS} DESTINATION bin)
endif()
