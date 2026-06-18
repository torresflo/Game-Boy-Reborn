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

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# SingleStepTests/sm83: external per-opcode JSON CPU test vectors (1000
# randomized cases per opcode, covering every legal unprefixed and
# CB-prefixed instruction). Large (several hundred MB), so it's gated behind
# an option and fetched into the build directory only - never committed to
# this repo.
option(GBR_ENABLE_JSON_CPU_TESTS "Fetch and run the SingleStepTests/sm83 external CPU test vectors" ON)
set(GBR_JSON_CPU_TEST_CASE_LIMIT 100 CACHE STRING "Max test cases to run per opcode JSON file (each file has 1000)")

if(GBR_ENABLE_JSON_CPU_TESTS)
    FetchContent_Declare(
        sm83_tests
        GIT_REPOSITORY https://github.com/SingleStepTests/sm83.git
        GIT_TAG main
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(sm83_tests)

    # The opcode JSON files live under v1/ as of this writing (e.g.
    # v1/00.json, v1/cb 00.json); locate it by anchoring on a file that must
    # exist so this keeps working if the repo's layout ever changes.
    file(GLOB_RECURSE SM83_OPCODE_00_ANCHOR "${sm83_tests_SOURCE_DIR}/*00.json")
    if(SM83_OPCODE_00_ANCHOR)
        list(GET SM83_OPCODE_00_ANCHOR 0 SM83_OPCODE_00_ANCHOR)
        get_filename_component(SM83_TEST_DATA_DIR "${SM83_OPCODE_00_ANCHOR}" DIRECTORY)
    else()
        message(WARNING "GBR_ENABLE_JSON_CPU_TESTS is ON but no opcode JSON files were found under ${sm83_tests_SOURCE_DIR}; JSON CPU tests will be skipped.")
    endif()
endif()

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(Game-Boy-Reborn-Tests ${TEST_SOURCES})
target_include_directories(Game-Boy-Reborn-Tests PRIVATE ".")
target_link_libraries(Game-Boy-Reborn-Tests PRIVATE GameBoyCore doctest::doctest nlohmann_json::nlohmann_json)
target_compile_features(Game-Boy-Reborn-Tests PUBLIC cxx_std_20)

if(SM83_TEST_DATA_DIR)
    target_compile_definitions(Game-Boy-Reborn-Tests PRIVATE
        SM83_TEST_DATA_DIR="${SM83_TEST_DATA_DIR}"
        GBR_JSON_CPU_TEST_CASE_LIMIT=${GBR_JSON_CPU_TEST_CASE_LIMIT}
    )
endif()

add_test(NAME UnitTests COMMAND Game-Boy-Reborn-Tests)
