# =============================================================================
# OpenSSL - Linux Platform
# =============================================================================

set(SSL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/x64")

# Find libraries
find_library(SSL_CRYPTO_LIB
    NAMES crypto libcrypto.so
    PATHS "${SSL_ROOT}/lib"
    NO_DEFAULT_PATH
)

find_library(SSL_SSL_LIB
    NAMES ssl libssl.so
    PATHS "${SSL_ROOT}/lib"
    NO_DEFAULT_PATH
)

# Fallback to system library if bundled not found
if(NOT SSL_CRYPTO_LIB OR NOT SSL_SSL_LIB)
    find_package(OpenSSL QUIET)
    if(OPENSSL_FOUND)
        message(STATUS "[OpenSSL] Using system library")
        add_library(openssl INTERFACE)
        target_link_libraries(openssl INTERFACE OpenSSL::SSL OpenSSL::Crypto)
        return()
    else()
        message(FATAL_ERROR "[OpenSSL] Library not found. Install with: sudo apt install libssl-dev")
    endif()
endif()

# Find shared libraries for copying
file(GLOB SSL_SHARED_LIBS "${SSL_ROOT}/lib/*.so*")

# Find include directory
set(SSL_INCLUDE "${SSL_ROOT}/include")
if(NOT EXISTS "${SSL_INCLUDE}/openssl/ssl.h")
    message(FATAL_ERROR "[OpenSSL] Headers not found in ${SSL_INCLUDE}")
endif()

message(STATUS "[OpenSSL] Linux x64")
message(STATUS "[OpenSSL] Crypto: ${SSL_CRYPTO_LIB}")
message(STATUS "[OpenSSL] SSL: ${SSL_SSL_LIB}")
message(STATUS "[OpenSSL] Shared libs: ${SSL_SHARED_LIBS}")
message(STATUS "[OpenSSL] Include: ${SSL_INCLUDE}")

# Create copy target for shared libraries
if(SSL_SHARED_LIBS)
    add_custom_target(copy_ssl)
    set_target_properties(copy_ssl PROPERTIES FOLDER framework/thirdparty/openssl)

    add_custom_command(TARGET copy_ssl POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SSL_SHARED_LIBS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        COMMAND_EXPAND_LISTS
        COMMENT "[OpenSSL] Copying shared libraries to output directory"
    )
endif()

# Create interface library
add_library(openssl INTERFACE)
target_include_directories(openssl INTERFACE "${SSL_INCLUDE}")
target_link_libraries(openssl INTERFACE "${SSL_CRYPTO_LIB}" "${SSL_SSL_LIB}")

if(TARGET copy_ssl)
    add_dependencies(openssl copy_ssl)
endif()

# Install shared libraries to installation directory
if(SSL_SHARED_LIBS)
    install(FILES ${SSL_SHARED_LIBS} DESTINATION bin)
endif()
