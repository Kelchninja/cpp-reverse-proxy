cmake_minimum_required(VERSION 3.20)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(HttpServerCPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Werror)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /WX)
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

option(BUILD_TESTS "Build unit tests" OFF)

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

# --- Processing DLL ---
# add_library(processing SHARED
#     processing/src/processing.cpp
# )
# target_include_directories(processing PUBLIC
#     common/include
# )
# target_link_libraries(processing PUBLIC config)


# --- Server EXE ---
add_executable(server
    server/src/main.cpp
    server/src/server.cpp
)

target_compile_features(server PRIVATE cxx_std_23)

target_include_directories(server PRIVATE
    server/include
    API/include
)
target_link_libraries(server PRIVATE llhttp asio::asio config)


# Verbindet beide: Server linkt gegen processing
# target_link_libraries(server PRIVATE processing)


# --- Tests ---
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)

    # Server
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

    # Verarbeitung (processing)
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

    # API
    add_executable(api_tests
        tests/api/api_tests.cpp
    )
    target_include_directories(api_tests PRIVATE
        API/include
    )
    target_link_libraries(api_tests PRIVATE
        GTest::gtest
        GTest::gtest_main
    )
    target_compile_features(api_tests PRIVATE cxx_std_23)

    # Security
    add_executable(security_tests
        tests/security/security_tests.cpp
    )
    target_include_directories(security_tests PRIVATE
        server/include
        API/include
    )
    target_link_libraries(security_tests PRIVATE
        GTest::gtest
        GTest::gtest_main
        config
    )
    target_compile_features(security_tests PRIVATE cxx_std_23)

    include(GoogleTest)
    gtest_discover_tests(server_tests)
    # gtest_discover_tests(processing_tests)
    gtest_discover_tests(api_tests)
    gtest_discover_tests(security_tests)
endif()