set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -w -O0 -g3 -no-pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_DEBUG} -w -O0 -g3 -no-pie")


package_add_test(integration_tests
        SOURCES DropFileServerIntegrationTests.cpp
        DEPENDS drop-file-shared-lib
        )