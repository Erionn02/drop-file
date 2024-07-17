set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -w -O0 -g3 -no-pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_DEBUG} -w -O0 -g3 -no-pie")


package_add_test(integration_tests
        SOURCES
        DropFileServerIntegrationTests.cpp
        ClientSocketTests.cpp
        MaliciousClientTests.cpp
        DEPENDS
        drop-file-client-lib
        drop-file-server-lib
        )

target_compile_definitions(integration_tests PRIVATE EXAMPLE_CERT_DIR="${CMAKE_SOURCE_DIR}/example_assets")

target_include_directories(integration_tests PRIVATE ../test_utils)
