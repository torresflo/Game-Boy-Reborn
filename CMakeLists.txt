cmake_minimum_required(VERSION 3.28)

# Project
set(ProjectName "Game-Boy-Reborn")
project(${ProjectName})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${ProjectName})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Select build system and set compile options
include(${CMAKE_CURRENT_LIST_DIR}/cmake/BuildSetting.cmake)

file (GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file (GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS "src/*.h")

set(CORE_SOURCES ${PROJECT_SOURCES})
list(REMOVE_ITEM CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# Core emulator code, shared by the executable and the test suite
add_library(GameBoyCore STATIC ${CORE_SOURCES})
target_include_directories(GameBoyCore PUBLIC "src")
target_compile_features(GameBoyCore PUBLIC cxx_std_20)

# Create executable file
add_executable(${ProjectName} "src/main.cpp")
target_link_libraries(${ProjectName} PRIVATE GameBoyCore)

# Fetch SFML and build it as a static library alongside the project
include(FetchContent)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SFML)

target_link_libraries(${ProjectName} PUBLIC sfml-system sfml-window sfml-graphics sfml-audio)

target_compile_features(${ProjectName} PUBLIC cxx_std_20)

message(STATUS "Sources: ${PROJECT_SOURCES}")

enable_testing()
add_subdirectory(tests)

# Building the main project also builds the test executable, so it's ready
# to run without needing a separate build step.
add_dependencies(${ProjectName} Game-Boy-Reborn-Tests)