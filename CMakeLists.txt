cmake_minimum_required(VERSION 3.15)
project(SNBot CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(spdlog CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(unofficial-libmariadb CONFIG REQUIRED)
# files
add_executable(SNBot
    # --- Source Files ---
    "src/main.cpp"
    "src/DBAgent.cpp"
    "src/Bot.cpp"
    "src/GameInfo.cpp"
    "src/Loader.cpp"
    "src/logger.cpp"

    # --- Header Files (Arayüzde görünmesi için şart) ---
    "include/DBAgent.h"
    "include/Bot.h"
    "include/GameInfo.h"
    "include/Loader.h"
    "include/logger.h"
)

# header files
target_include_directories(SNBot PRIVATE include)
target_compile_definitions(SNBot PRIVATE CONCPP_BUILD_SHARED)

target_link_libraries(SNBot PRIVATE
    unofficial::libmariadb
    spdlog::spdlog
    CURL::libcurl
)