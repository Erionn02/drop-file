include(CMakePrintHelpers)

add_subdirectory(client)
add_subdirectory(server)

add_lib(drop-file-shared-lib
        SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/SocketBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InitSessionMessage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Utils.cpp
        )
