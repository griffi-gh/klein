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

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

# vendored deps:
add_subdirectory(vendor/SFML)
add_subdirectory(vendor/entt)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
  src/*.cpp
)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src/)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_link_libraries(${PROJECT_NAME} PRIVATE
  sfml-graphics
  sfml-window
  sfml-system
  EnTT::EnTT
)
target_compile_options(${PROJECT_NAME} PRIVATE
  -Wall
  -fdiagnostics-color=always
  $<$<CONFIG:Debug>:-Og>
)
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
  PUBLIC
    $<$<CONFIG:Debug>:ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/">
    $<$<NOT:$<CONFIG:Debug>>:ASSETS_PATH="./assets/">
)
