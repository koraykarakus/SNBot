cmake_minimum_required(VERSION 3.15)
project(SNBot CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(unofficial-libmariadb CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
# files
add_executable(SNBot
    # --- Source Files ---
    "src/main.cpp"
    "src/CDatabase.cpp"
    "src/CHandleBuildings.cpp" 
    "src/CHandleResourceUpdate.cpp" 
    "src/CHandleColonization.cpp"
    "src/CLogger.cpp"
    "src/CBotManager.cpp"
    "src/CCommandHandler.cpp" 
    "src/CHandleConsoleCommand.cpp" 
    "src/CHandleResearches.cpp"
    "src/CApplication.cpp"
    "src/CLanguage.cpp"
    "deps/bcrypt/src/bcrypt.cpp"
    "deps/bcrypt/src/blowfish.cpp"

    # --- Header Files ---
    "include/CDatabase.h"
    "include/CLogger.h"
    "include/table_planets.h"  
    "include/CBotManager.h" 
    "include/table_users.h"  
    "include/enums.h" 
    "include/globals.h" 
    "include/table_fleets.h" 
    "include/CApplication.h" 
    "include/CCommandHandler.h" 
    "include/CLanguage.h" 
    "include/bot_names.h"
    "deps/bcrypt/include/bcrypt.h"
)

# additional include directories
target_include_directories(SNBot PRIVATE 
    include
    deps/bcrypt/include
    deps/tomlplusplus/include
    #${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include
)
target_compile_definitions(SNBot PRIVATE CONCPP_BUILD_SHARED)

target_link_libraries(SNBot PRIVATE
    unofficial::libmariadb
    spdlog::spdlog
)

# coppy language files from root to build folder
add_custom_command(TARGET SNBot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/language
    $<TARGET_FILE_DIR:SNBot>/language
)

add_compile_options(/utf-8)