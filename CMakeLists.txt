cmake_minimum_required(VERSION 3.20)
project(HttpServerProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Processing DLL ---
add_library(processing SHARED
    processing/src/processing.cpp
)
target_include_directories(processing PUBLIC
    common/include
)

# --- Server EXE ---
add_executable(server
    server/src/main.cpp
)
target_include_directories(server PRIVATE
    common/include
)

# Verbindet beide: Server linkt gegen processing
target_link_libraries(server PRIVATE processing)