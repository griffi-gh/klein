cmake_minimum_required(VERSION 3.28)

project(vendor NONE)

set(CMAKE_FOLDER "vendor")

# hack for msvc/vs to not show vendor target sln files
function(set_folder_recursive dir folder)
  get_property(targets DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
  foreach(t ${targets})
    set_target_properties(${t} PROPERTIES FOLDER "${folder}")
  endforeach()
  get_property(subs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
  foreach(s ${subs})
    set_folder_recursive("${s}" "${folder}")
  endforeach()
endfunction()
set_folder_recursive("${CMAKE_CURRENT_SOURCE_DIR}/" "vendor")

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui" CACHE PATH "" FORCE)

set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
add_subdirectory(SFML EXCLUDE_FROM_ALL SYSTEM)

set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(zlib EXCLUDE_FROM_ALL SYSTEM)

set(SPDLOG_USE_STD_FORMAT ON CACHE BOOL "" FORCE)
set(SPDLOG_NO_EXCEPTIONS ON CACHE BOOL "" FORCE)
add_subdirectory(spdlog EXCLUDE_FROM_ALL SYSTEM)

set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
add_subdirectory(imgui-sfml EXCLUDE_FROM_ALL SYSTEM)

add_subdirectory(entt EXCLUDE_FROM_ALL SYSTEM)
add_subdirectory(nlohmann_json EXCLUDE_FROM_ALL SYSTEM)

unset(CMAKE_FOLDER)
