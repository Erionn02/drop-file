add_lib(drop-file-server-lib SOURCES
        ServerSideClientSession.cpp
        SessionsManager.cpp
        ServerArgParser.cpp
        DEPENDS
        drop-file-shared-lib
        )