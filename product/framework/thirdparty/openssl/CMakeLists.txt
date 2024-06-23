set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/windows/x64")
find_package(OpenSSL  REQUIRED)

add_custom_target(copy_ssl)
set_target_properties(copy_ssl PROPERTIES FOLDER thirdparty/openssl)

add_custom_command( TARGET copy_ssl POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy ${OPENSSL_ROOT_DIR}/bin/libcrypto-1_1-x64.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/libcrypto-1_1-x64.dll)

add_custom_command( TARGET copy_ssl PRE_BUILD
COMMAND ${CMAKE_COMMAND} -E copy ${OPENSSL_ROOT_DIR}/bin/libssl-1_1-x64.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/libssl-1_1-x64.dll)


add_library(openssl INTERFACE)
target_include_directories(openssl INTERFACE ${OPENSSL_ROOT_DIR}/include)
target_link_libraries(openssl INTERFACE OpenSSL::Crypto OpenSSL::SSL)
add_dependencies(openssl copy_ssl)