# =============================================================================
# OpenSSL - Android Platform
# =============================================================================

# Determine ABI directory
if(NOT ANDROID_ABI)
    message(FATAL_ERROR "[OpenSSL] ANDROID_ABI not defined")
endif()

set(SSL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${ANDROID_ABI}")

if(NOT EXISTS "${SSL_ROOT}")
    message(FATAL_ERROR "[OpenSSL] Android ABI directory not found: ${SSL_ROOT}")
endif()

# Find libraries (shared libraries in lib/)
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

if(NOT SSL_CRYPTO_LIB OR NOT SSL_SSL_LIB)
    message(FATAL_ERROR "[OpenSSL] Libraries not found in ${SSL_ROOT}/lib")
endif()

# Find include directory
set(SSL_INCLUDE "${SSL_ROOT}/include")
if(NOT EXISTS "${SSL_INCLUDE}/openssl/ssl.h")
    message(FATAL_ERROR "[OpenSSL] Headers not found in ${SSL_INCLUDE}")
endif()

message(STATUS "[OpenSSL] Android ${ANDROID_ABI}")
message(STATUS "[OpenSSL] Crypto: ${SSL_CRYPTO_LIB}")
message(STATUS "[OpenSSL] SSL: ${SSL_SSL_LIB}")
message(STATUS "[OpenSSL] Include: ${SSL_INCLUDE}")

# Create interface library
add_library(openssl INTERFACE)
target_include_directories(openssl INTERFACE "${SSL_INCLUDE}")
target_link_libraries(openssl INTERFACE "${SSL_CRYPTO_LIB}" "${SSL_SSL_LIB}")
