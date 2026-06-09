cmake_minimum_required(VERSION 3.14)

# Project
set(ProjectName "Project-Name")
project(${ProjectName})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${ProjectName})

# Select build system and set compile options
include(${CMAKE_CURRENT_LIST_DIR}/cmake/BuildSetting.cmake)

file (GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file (GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS "src/*.h")

# Create executable file
add_executable(${ProjectName} ${PROJECT_SOURCES})
target_include_directories(${ProjectName} PRIVATE "src")

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