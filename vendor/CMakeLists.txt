cmake_minimum_required(VERSION 3.28)

project(vendor NONE)

add_subdirectory(SFML EXCLUDE_FROM_ALL SYSTEM)
add_subdirectory(entt EXCLUDE_FROM_ALL SYSTEM)
add_subdirectory(nlohmann_json EXCLUDE_FROM_ALL SYSTEM)
add_subdirectory(zlib EXCLUDE_FROM_ALL SYSTEM)
add_subdirectory(spdlog EXCLUDE_FROM_ALL SYSTEM)
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui" CACHE PATH "" FORCE)
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
add_subdirectory(imgui-sfml EXCLUDE_FROM_ALL SYSTEM)
