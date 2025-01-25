include(CMakePrintHelpers)

function(package_add_test TESTNAME)
    cmake_parse_arguments(ARGS "" "" "SOURCES;DEPENDS" ${ARGN})
    add_executable(${TESTNAME} ${ARGS_SOURCES})
    target_include_directories(${TESTNAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(
            ${TESTNAME}
            ${CONAN_LIBS}
            ${CMAKE_DL_LIBS}
            ${ARGS_DEPENDS}
    )
    gtest_discover_tests(${TESTNAME}
            WORKING_DIRECTORY ${PROJECT_DIR}
            PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
            )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER ${CMAKE_SOURCE_DIR}/tests)
endfunction()

function(add_app APP_NAME)
    cmake_parse_arguments(ARGS "" "" "SOURCES;DEPENDS" ${ARGN})
    add_executable(${APP_NAME} ${ARGS_SOURCES})
    target_link_libraries(${APP_NAME} PRIVATE "${ARGS_DEPENDS}")
    set_link_options(${APP_NAME})
endfunction()

function(add_lib LIB_NAME)
    cmake_parse_arguments(ARGS "" "" "SOURCES;DEPENDS" ${ARGN})
    add_library(${LIB_NAME} ${ARGS_SOURCES})
    target_link_libraries(${LIB_NAME} PRIVATE "${ARGS_DEPENDS}")
    set_link_options(${LIB_NAME})
endfunction()

function(set_link_options TARGET_NAME)
    target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${TARGET_NAME} PRIVATE ${CONAN_LIBS})
    target_compile_options(${TARGET_NAME} PRIVATE
            -Wno-unused-variable
            -Wno-maybe-uninitialized
            -Werror
            -Wall
            -Wextra
            -Wnon-virtual-dtor
            -Wcast-align
            -Wunused
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wmisleading-indentation
            -Wduplicated-cond
            -Wduplicated-branches
            -Wlogical-op
            -Wnull-dereference
            -Wuseless-cast
            -Wdouble-promotion
            )
    target_link_options(${TARGET_NAME} PRIVATE -rdynamic)
endfunction()

macro(setup_conan)
    if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan/conan.cmake")
        message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
        file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/refs/heads/develop2/conan_provider.cmake"
                "${CMAKE_BINARY_DIR}/conan/conan.cmake")
    endif ()
    list(APPEND CMAKE_PROJECT_TOP_LEVEL_INCLUDES "${CMAKE_BINARY_DIR}/conan/conan.cmake")
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/Release/generators")
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/Debug/generators")
    set(CONAN_SYSTEM_INCLUDES ON)
endmacro()

function(download_conan_cmake)
    if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan/conan.cmake")
        message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
        file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan/conan.cmake")
    endif ()
endfunction()
