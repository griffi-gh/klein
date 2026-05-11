cmake_minimum_required(VERSION 3.28)

project(klein VERSION 1.0 LANGUAGES CXX)

if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    # HACK: avoid /RTC1 in debug builds
    cmake_policy(SET CMP0184 NEW)
    set(CMAKE_MSVC_RUNTIME_CHECKS "")
    foreach(flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REGEX REPLACE "/RTC(su|[1su])" "" ${flag_var} "${${flag_var}}")
    endforeach(flag_var)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_CLANG_TIDY "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

if(NOT MSVC)
  add_compile_options(-fdiagnostics-color=always)
endif()

# vendored deps:
add_subdirectory(vendor EXCLUDE_FROM_ALL SYSTEM)

target_compile_definitions(EnTT INTERFACE
    ENTT_STANDARD_CPP
    ENTT_USE_ATOMIC
    ENTT_ID_TYPE=uint64_t)

# project
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
  src/*.cpp
  src/*.cc)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src/)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_link_libraries(${PROJECT_NAME} PRIVATE
  SFML::Graphics
  SFML::Window
  SFML::System
  EnTT::EnTT
  nlohmann_json::nlohmann_json
  ZLIB::ZLIB
  spdlog::spdlog
  ImGui-SFML::ImGui-SFML
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Debug>:ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets">
    $<$<NOT:$<CONFIG:Debug>>:ASSETS_PATH="./assets">
)

target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-O1>)
if (MSVC)
    # disable console window in release builds
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:-g>)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
install(FILES $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> DESTINATION .)

if(WIN32)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
      $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS
  )
endif()

install(FILES CREDITS.md
  DESTINATION .
  CONFIGURATIONS Release RelWithDebInfo MinSizeRel)
install(DIRECTORY assets/
  DESTINATION assets
  CONFIGURATIONS Release RelWithDebInfo MinSizeRel)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:myapp>/assets
    )
endif()

