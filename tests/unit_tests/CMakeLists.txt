set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -w -O0 -g3 -no-pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_DEBUG} -w -O0 -g3 -no-pie")

package_add_test(unit_tests
        SOURCES
        InitSessionMessageTests.cpp
        DirectoryCompressorTests.cpp
        UtilsTests.cpp
        ClientArgParserTests.cpp
        ServerArgParserTests.cpp
        SessionsManagerTests.cpp
        FSEntryInfoTests.cpp
        GzipTests.cpp
        DEPENDS drop-file-shared-lib
        )
target_include_directories(unit_tests PRIVATE ../test_utils)