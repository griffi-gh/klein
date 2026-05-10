cmake_minimum_required(VERSION 3.28)

project(klein VERSION 1.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

if(MSVC)
  add_compile_options(
    $<$<CONFIG:>:/MT>
    $<$<CONFIG:Debug>:/MTd>
    $<$<CONFIG:Release>:/MT>
  )
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_CLANG_TIDY "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

add_compile_options(-fdiagnostics-color=always)
add_compile_definitions($<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)

set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
add_compile_definitions(
  ENTT_STANDARD_CPP
  ENTT_USE_ATOMIC
  ENTT_ID_TYPE=uint64_t
)
set(SPDLOG_USE_STD_FORMAT ON CACHE BOOL "" FORCE)
set(SPDLOG_NO_EXCEPTIONS ON CACHE BOOL "" FORCE)
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui" CACHE PATH "" FORCE)

# vendored deps:
add_subdirectory(vendor EXCLUDE_FROM_ALL)

# project
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
  src/*.cpp
  src/*.cc)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src/)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_link_libraries(${PROJECT_NAME} PRIVATE
  sfml-graphics
  sfml-window
  sfml-system
  EnTT::EnTT
  nlohmann_json::nlohmann_json
  zlib
  spdlog
  ImGui-SFML::ImGui-SFML
)
target_compile_options(${PROJECT_NAME} PRIVATE
  -Wall
  $<$<CONFIG:Debug>:-Og>
)
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
  PUBLIC
    $<$<CONFIG:Debug>:ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets">
    $<$<NOT:$<CONFIG:Debug>>:ASSETS_PATH="./assets">
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

install(FILES $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> TYPE BIN)
