# =============================================================================
# OpenSSL - iOS Platform
# =============================================================================

set(SSL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# iOS uses XCFrameworks
set(SSL_CRYPTO_XCFRAMEWORK "${SSL_ROOT}/libcrypto.xcframework")
set(SSL_SSL_XCFRAMEWORK "${SSL_ROOT}/libssl.xcframework")

if(NOT EXISTS "${SSL_CRYPTO_XCFRAMEWORK}")
    message(FATAL_ERROR "[OpenSSL] libcrypto.xcframework not found: ${SSL_CRYPTO_XCFRAMEWORK}")
endif()

if(NOT EXISTS "${SSL_SSL_XCFRAMEWORK}")
    message(FATAL_ERROR "[OpenSSL] libssl.xcframework not found: ${SSL_SSL_XCFRAMEWORK}")
endif()

# Find include directory
set(SSL_INCLUDE "${SSL_ROOT}/include")
if(NOT EXISTS "${SSL_INCLUDE}/openssl/ssl.h")
    message(FATAL_ERROR "[OpenSSL] Headers not found in ${SSL_INCLUDE}")
endif()

message(STATUS "[OpenSSL] iOS")
message(STATUS "[OpenSSL] Crypto XCFramework: ${SSL_CRYPTO_XCFRAMEWORK}")
message(STATUS "[OpenSSL] SSL XCFramework: ${SSL_SSL_XCFRAMEWORK}")
message(STATUS "[OpenSSL] Include: ${SSL_INCLUDE}")

# Create interface library for iOS
add_library(openssl INTERFACE)
target_include_directories(openssl INTERFACE "${SSL_INCLUDE}")
target_link_libraries(openssl INTERFACE 
    "${SSL_CRYPTO_XCFRAMEWORK}"
    "${SSL_SSL_XCFRAMEWORK}"
)
