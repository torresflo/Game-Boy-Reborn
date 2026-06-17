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

# Find SFML
set(SFML_STATIC_LIBRARIES TRUE)

if(MSVC_VERSION)
    set(SFML_ROOT "C:/SFML-2.6.1/")
endif()

find_package(SFML 2.6 COMPONENTS system window graphics audio REQUIRED)

# If found, include and link; otherwise output error message
if(SFML_FOUND)
	target_link_libraries(${ProjectName} PUBLIC sfml-system sfml-window sfml-graphics sfml-audio )	
else()
	set(SFML_ROOT "" CACHE PATH "SFML top-level directory")
	message("\n-> SFML directory not found. Set SFML_ROOT to SFML's top-level path (containing \"include\" and \"lib\" directories).")
	message("-> Make sure the SFML libraries with the same configuration (Release/Debug, Static/Dynamic) exist.\n")
endif()

target_compile_features(${ProjectName} PUBLIC cxx_std_20)

message(STATUS "Sources: ${PROJECT_SOURCES}")

enable_testing()
add_subdirectory(tests)

# Building the main project also builds the test executable, so it's ready
# to run without needing a separate build step.
add_dependencies(${ProjectName} Game-Boy-Reborn-Tests)