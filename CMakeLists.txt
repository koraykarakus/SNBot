cmake_minimum_required(VERSION 3.15)
project(SNBot CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(spdlog CONFIG REQUIRED)
find_package(unofficial-libmariadb CONFIG REQUIRED)
# files
add_executable(SNBot
    # --- Source Files ---
    "src/main.cpp"
    "src/CDatabase.cpp"
    
    
    "src/CLoader.cpp"
    "src/CLogger.cpp"

    # --- Header Files (Arayüzde görünmesi için şart) ---
    "include/CDatabase.h"
    
    
    "include/CLoader.h"
    "include/CLogger.h"
 "include/table_planets.h"  "include/CBotManager.h" "src/CBotManager.cpp"  "include/table_users.h"  "include/enums.h" "include/globals.h" "src/CHandleBuildings.cpp" "src/CHandleResourceUpdate.cpp" "src/CHandleColonization.cpp" "include/table_fleets.h")

# header files
target_include_directories(SNBot PRIVATE include)
target_compile_definitions(SNBot PRIVATE CONCPP_BUILD_SHARED)

target_link_libraries(SNBot PRIVATE
    unofficial::libmariadb
    spdlog::spdlog
)