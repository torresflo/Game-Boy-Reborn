include(FetchContent)
# doctest's own CMakeLists.txt declares a cmake_minimum_required older than
# what this CMake version still supports; tell FetchContent to treat it as
# this version instead of failing the configure step.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
)
FetchContent_MakeAvailable(doctest)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(Game-Boy-Reborn-Tests ${TEST_SOURCES})
target_include_directories(Game-Boy-Reborn-Tests PRIVATE ".")
target_link_libraries(Game-Boy-Reborn-Tests PRIVATE GameBoyCore doctest::doctest)
target_compile_features(Game-Boy-Reborn-Tests PUBLIC cxx_std_20)

add_test(NAME UnitTests COMMAND Game-Boy-Reborn-Tests)
