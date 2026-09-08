cmake_minimum_required(VERSION 3.20)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(HttpServerCPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(PROJECT_WARNING_FLAGS -Wall -Wextra -Werror)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(PROJECT_WARNING_FLAGS /W4 /WX)
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

option(BUILD_TESTS "Build unit tests" ON)

include(FetchContent)

# --- packages ---
find_package(asio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# --- llhttp (replaces the old http-parser submodule) ---
FetchContent_Declare(llhttp
  URL "https://github.com/nodejs/llhttp/archive/refs/tags/release/v9.4.3.tar.gz"
  DOWNLOAD_EXTRACT_TIMESTAMP true)

set(LLHTTP_BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(LLHTTP_BUILD_STATIC_LIBS ON CACHE INTERNAL "")
FetchContent_MakeAvailable(llhttp)
# --- end llhttp ---

# --- config (ServerConfig, SecurityConfig, ProcessingConfig, all in one file) ---
add_library(config STATIC
    config/src/config.cpp
)
target_include_directories(config PUBLIC
    config/include
)
target_link_libraries(config PUBLIC nlohmann_json::nlohmann_json)
target_compile_features(config PUBLIC cxx_std_23)
target_compile_options(config PRIVATE ${PROJECT_WARNING_FLAGS})

# --- Processing .dll / .dylib / .so ---
add_library(processing SHARED
    processing/src/processing.cpp
)

target_include_directories(processing PUBLIC
    processing/include
    common
)

target_link_libraries(processing PRIVATE config)

target_compile_options(processing PRIVATE
    $<$<NOT:$<PLATFORM_ID:Windows>>:-fvisibility=hidden>
)

target_compile_definitions(processing PRIVATE PROCESSING_EXPORTS)


# --- Server EXE ---
add_executable(server
    server/src/main.cpp
    server/src/server.cpp
    server/src/connection.cpp
)

target_compile_features(server PRIVATE cxx_std_23)
target_compile_options(server PRIVATE ${PROJECT_WARNING_FLAGS})

target_include_directories(server PRIVATE
    server/include
    API/include
    common
)
target_link_libraries(server PRIVATE llhttp::llhttp asio::asio config)

# --- config.json neben die server-Executable kopieren ---
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/bin/config.json
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/config.json
            ${CMAKE_BINARY_DIR}/bin/config.json
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config.json
    COMMENT "Kopiere config.json"
    VERBATIM
)
add_custom_target(server_config DEPENDS ${CMAKE_BINARY_DIR}/bin/config.json)

add_dependencies(server server_config)
# add_dependencies(processing server_config)
# add_dependencies(security server_config)

# Verbindet beide: Server linkt gegen processing
# target_link_libraries(server PRIVATE processing)


# --- Tests ---
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)

    # --- Server ---

    add_executable(server_tests
        tests/server/server_tests.cpp
    )
    target_include_directories(server_tests PRIVATE
        server/include
        API/include
    )
    target_link_libraries(server_tests PRIVATE
        llhttp
        asio::asio
        config
        GTest::gtest
        GTest::gtest_main
    )
    target_compile_features(server_tests PRIVATE cxx_std_23)
    target_compile_options(server_tests PRIVATE ${PROJECT_WARNING_FLAGS})

    # --- Verarbeitung (processing) ---
    
    # add_executable(processing_tests
    #     tests/processing/processing_tests.cpp
    # )
    # target_include_directories(processing_tests PRIVATE
    #     common/include
    # )
    # target_link_libraries(processing_tests PRIVATE
    #     processing
    #     config
    #     GTest::gtest
    #     GTest::gtest_main
    # )
    # target_compile_features(processing_tests PRIVATE cxx_std_23)

    # --- API ---

    # add_executable(api_tests
    #     tests/api/api_tests.cpp
    # )
    # target_include_directories(api_tests PRIVATE
    #     API/include
    # )
    # target_link_libraries(api_tests PRIVATE
    #     GTest::gtest
    #     GTest::gtest_main
    # )
    # target_compile_features(api_tests PRIVATE cxx_std_23)
    # target_compile_options(api_tests PRIVATE ${PROJECT_WARNING_FLAGS})

    # --- Security ---

    # add_executable(security_tests
    #     tests/security/security_tests.cpp
    # )
    # target_include_directories(security_tests PRIVATE
    #     server/include
    #     API/include
    # )
    # target_link_libraries(security_tests PRIVATE
    #     GTest::gtest
    #     GTest::gtest_main
    #     config
    # )
    # target_compile_features(security_tests PRIVATE cxx_std_23)
    # target_compile_options(security_tests PRIVATE ${PROJECT_WARNING_FLAGS})

    # --- Config ---

    add_executable(config_tests 
        tests/config/config_tests.cpp
    )
    target_include_directories(config_tests PRIVATE
        config/include
    )
    target_link_libraries(config_tests PRIVATE
        GTest::gtest
        GTest::gtest_main
        config
    )
    target_compile_features(config_tests PRIVATE cxx_std_23)
    target_compile_options(config_tests PRIVATE ${PROJECT_WARNING_FLAGS})

    # --- Test-Executables nach test_bin statt bin ---
    set_target_properties(
        server_tests
        # processing_tests
        # api_tests
        # security_tests
        config_tests
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test_bin
    )

    include(GoogleTest)
    gtest_discover_tests(server_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test_bin)
    # gtest_discover_tests(processing_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test_bin)
    # gtest_discover_tests(api_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test_bin)
    # gtest_discover_tests(security_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test_bin)
    gtest_discover_tests(config_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/test_bin)
endif()