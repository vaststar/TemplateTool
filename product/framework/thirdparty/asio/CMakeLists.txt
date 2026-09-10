find_package(Threads REQUIRED)

# Standalone Asio is header-only. Keep all of its usage requirements on this
# interface target so first-party modules do not depend on repository paths or
# repeat platform-specific definitions.
add_library(asio INTERFACE)

target_include_directories(asio
    SYSTEM INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/asio-1.38.2/include
)

target_compile_definitions(asio
    INTERFACE
        ASIO_STANDALONE
        ASIO_NO_DEPRECATED
)

target_link_libraries(asio
    INTERFACE
        Threads::Threads
)

if(WIN32)
    target_compile_definitions(asio
        INTERFACE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
    )

    target_link_libraries(asio
        INTERFACE
            ws2_32
            mswsock
    )
endif()

set_target_properties(asio PROPERTIES
    FOLDER framework/thirdparty/asio
)
