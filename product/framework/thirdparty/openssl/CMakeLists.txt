set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/windows/x64")
find_package(OpenSSL  REQUIRED)

set(SSL_DLL_FILES)
file(GLOB SSL_DLL_FILES ${OPENSSL_ROOT_DIR}/bin/*.dll)

add_custom_target(copy_ssl)
set_target_properties(copy_ssl PROPERTIES FOLDER thirdparty/openssl)

add_custom_command( TARGET copy_ssl POST_BUILD  COMMAND_EXPAND_LISTS
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
    COMMAND ${CMAKE_COMMAND} -E copy "$<$<CONFIG:Release>:${SSL_DLL_FILES}>$<$<CONFIG:Debug>:${SSL_DLL_FILES}>" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>/"
)  


add_library(openssl INTERFACE)
target_include_directories(openssl INTERFACE ${OPENSSL_ROOT_DIR}/include)
target_link_libraries(openssl INTERFACE OpenSSL::Crypto OpenSSL::SSL)
add_dependencies(openssl copy_ssl)