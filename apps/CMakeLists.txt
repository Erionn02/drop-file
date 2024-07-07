include(CMakePrintHelpers)

add_app(drop-file-server
        drop-file-server.cpp
        )

add_app(drop-file
        drop-file.cpp
        )

target_compile_definitions(drop-file PRIVATE CERTS_DIR="${CMAKE_SOURCE_DIR}/example_assets")
target_compile_definitions(drop-file-server PRIVATE CERTS_DIR="${CMAKE_SOURCE_DIR}/example_assets")