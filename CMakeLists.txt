cmake_minimum_required(VERSION 3.28)

# Project
set(ProjectName "Game-Boy-Reborn")
project(${ProjectName})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${ProjectName})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Select build system and set compile options
include(${CMAKE_CURRENT_LIST_DIR}/cmake/BuildSetting.cmake)

file (GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "src/Core/*.cpp")
file (GLOB_RECURSE CORE_HEADERS CONFIGURE_DEPENDS "src/Core/*.h")

# Core emulator code, shared by the executable and the test suite
add_library(GameBoyCore STATIC ${CORE_SOURCES})
target_include_directories(GameBoyCore PUBLIC "src/Core")
target_compile_features(GameBoyCore PUBLIC cxx_std_20)

include(FetchContent)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# SFML: windowing, input, audio and pixel blit for the UI layer. The Network
# module (and its libssh2/mbedtls dependencies, unused by this emulator) is
# disabled - it doesn't build cleanly under this project's /W4 /WX.
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SFML)

# Dear ImGui has no CMake support upstream; just fetch its sources and point
# imgui-sfml at them via IMGUI_DIR (it compiles them into its own target).
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.9b
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)
set(IMGUI_DIR ${imgui_SOURCE_DIR} CACHE PATH "" FORCE)

# imgui-sfml: SFML 3 renderer/platform backend for Dear ImGui.
# Reuse the SFML targets already fetched above instead of calling find_package(SFML).
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    imgui-sfml
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG v3.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui-sfml)
if(MSVC)
    # imgui-sfml's touch handling still calls SFML's deprecated sf::Touch API;
    target_compile_options(ImGui-SFML PRIVATE /wd4996)
endif()

FetchContent_Declare(
    ImGuiFileDialog
    GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog.git
    GIT_TAG v0.6.8
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(ImGuiFileDialog)
target_link_libraries(ImGuiFileDialog PUBLIC ImGui-SFML)
if(MSVC)
    # Vendored code triggers a harmless int->char narrowing warning under this project's /WX.
    target_compile_options(ImGuiFileDialog PRIVATE /wd4244)
endif()

FetchContent_Declare(
    imgui_club
    GIT_REPOSITORY https://github.com/ocornut/imgui_club.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui_club)

# UI Sources
file (GLOB_RECURSE UI_SOURCES CONFIGURE_DEPENDS "src/UI/*.cpp")
file (GLOB_RECURSE UI_HEADERS CONFIGURE_DEPENDS "src/UI/*.h")

add_library(GameBoyUI STATIC ${UI_SOURCES})
target_include_directories(GameBoyUI PUBLIC "src/UI")
target_include_directories(GameBoyUI PUBLIC "${imgui_club_SOURCE_DIR}/imgui_memory_editor")
target_link_libraries(GameBoyUI PUBLIC GameBoyCore ImGui-SFML ImGuiFileDialog SFML::Audio)
target_compile_features(GameBoyUI PUBLIC cxx_std_20)

# Create executable file
add_executable(${ProjectName} WIN32 "src/main.cpp")
target_link_libraries(${ProjectName} PRIVATE GameBoyUI SFML::Main)
target_compile_features(${ProjectName} PUBLIC cxx_std_20)

message(STATUS "Sources: ${CORE_SOURCES}")

enable_testing()
add_subdirectory(tests)

# Building the main project also builds the test executable, so it's ready
# to run without needing a separate build step.
add_dependencies(${ProjectName} Game-Boy-Reborn-Tests)