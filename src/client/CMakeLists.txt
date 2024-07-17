add_lib(drop-file-client-lib SOURCES
        ClientSocket.cpp
        ArchiveManager.cpp
        ClientArgParser.cpp
        DropFileSendClient.cpp
        DropFileReceiveClient.cpp
        zstd.cpp
        FSEntryInfo.cpp
        DEPENDS
        drop-file-shared-lib
        )