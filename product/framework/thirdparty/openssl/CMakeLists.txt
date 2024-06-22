set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/windows/x64")
MESSAGE(STATUS "AAA${OPENSSL_ROOT_DIR}")

find_package(OpenSSL  REQUIRED)


add_custom_target(copy_ssl)
set_target_properties(copy_ssl PROPERTIES FOLDER thirdparty/openssl)
add_custom_command( TARGET copy_ssl PRE_BUILD
COMMAND ${CMAKE_COMMAND} -E copy ${OPENSSL_ROOT_DIR}/bin/libcrypto-1_1-x64.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)

add_custom_command( TARGET copy_ssl PRE_BUILD
COMMAND ${CMAKE_COMMAND} -E copy ${OPENSSL_ROOT_DIR}/bin/libssl-1_1-x64.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)

# include(BuildInterface)
# BuildInterface(
#     MODULE_NAME 
#         openssl
#     TARGET_DEPENDENICES_PUBLIC
#         copy_ssl
#     TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
#         ${OPENSSL_ROOT_DIR}/include
# )

add_library(openssl INTERFACE)
target_include_directories(openssl INTERFACE ${OPENSSL_ROOT_DIR}/include)
target_link_libraries(openssl INTERFACE OpenSSL::Crypto OpenSSL::SSL)
add_dependencies(openssl copy_ssl)