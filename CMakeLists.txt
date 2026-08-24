cmake_minimum_required(VERSION 3.20)
project(HttpServerCPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -fmax-errors=3")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g -DNDEBUG")

# --- Processing DLL ---
# add_library(processing SHARED
#     processing/src/processing.cpp
# )
# target_include_directories(processing PUBLIC
#     common/include
# )

# --- Server EXE ---
add_executable(server
    server/src/main.cpp
)
target_include_directories(server PRIVATE
    API/include
)

# Verbindet beide: Server linkt gegen processing
# target_link_libraries(server PRIVATE processing)