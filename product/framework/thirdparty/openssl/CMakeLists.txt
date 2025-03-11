if(WIN32)
    set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/windows/x64")
elseif(APPLE)
    set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/macosx/macosx_x86_64_release")
endif()

find_package(OpenSSL REQUIRED)

set(SSL_DLL_FILES)
if(WIN32)
    file(GLOB SSL_DLL_FILES ${OPENSSL_ROOT_DIR}/bin/*.dll)
elseif(APPLE)
    file(GLOB SSL_DLL_FILES ${OPENSSL_ROOT_DIR}/lib/*.dylib)
endif()



add_library(openssl INTERFACE)
target_include_directories(openssl INTERFACE ${OPENSSL_ROOT_DIR}/include)
target_link_libraries(openssl INTERFACE OpenSSL::Crypto OpenSSL::SSL)


if (WIN32)
add_custom_target(copy_ssl)
set_target_properties(copy_ssl PROPERTIES FOLDER thirdparty/openssl)

add_custom_command( TARGET copy_ssl POST_BUILD  COMMAND_EXPAND_LISTS
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
    COMMAND ${CMAKE_COMMAND} -E copy "$<$<CONFIG:Release>:${SSL_DLL_FILES}>$<$<CONFIG:Debug>:${SSL_DLL_FILES}>" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>/"
)  
add_dependencies(openssl copy_ssl)
endif()