include(CMakePrintHelpers)

add_app(drop-file-server
        SOURCES
        drop-file-server.cpp
        DEPENDS
        drop-file-server-lib
        )

add_app(drop-file
        SOURCES
        drop-file.cpp
        DEPENDS
        drop-file-client-lib
        )
