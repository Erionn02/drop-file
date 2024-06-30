include(CMakePrintHelpers)

add_app(server
        drop-file-server.cpp
        )

add_app(client
        drop-file.cpp
        )