include(CMakePrintHelpers)


add_lib(drop-file-shared-lib
        SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/SocketBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InitSessionMessage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DropFileServer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ServerSideClientSession.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SessionsManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ClientSession.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DirectoryCompressor.cpp
        )
