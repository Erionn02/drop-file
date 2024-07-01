include(CMakePrintHelpers)


add_lib(drop-file-shared-lib
        SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/SocketBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/InitSessionMessage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DropFileServer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ServerSideClientSession.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SessionsManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ClientSocket.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DirectoryCompressor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ArgParser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DropFileSendClient.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DropFileReceiveClient.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FileHash.cpp
        )
