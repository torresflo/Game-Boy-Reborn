// Data-driven CPU correctness tests backed by the external SingleStepTests/sm83
// JSON test vectors (one file per opcode, randomized initial/final CPU state and
// a cycle-by-cycle bus access log). Fetched by CMake into the build directory;
// this file compiles to nothing when GBR_ENABLE_JSON_CPU_TESTS is OFF or the
// fetched repository's layout couldn't be located (see tests/CMakeLists.txt).
#ifdef SM83_TEST_DATA_DIR

#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <set>
#include <string>

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

#include "CentralProcessingUnitJsonTestFixture.h"

namespace
{
    using nlohmann::json;

    // Opcodes that this codebase maps to InstructionType::NONE (illegal on
    // real SM83 hardware). Dispatching one calls exit(-1) (see
    // CentralProcessingUnit_Instructions.cpp), which would kill the whole
    // test process, so they're excluded rather than expected to fail a case.
    // 0xCB itself has no standalone JSON file either - it's the CB-prefix
    // byte, exercised entirely through the separate "cb XX.json" files.
    const std::set<u8> SkippedUnprefixedOpcodes = {0xCB, 0xD3, 0xDB, 0xDD, 0xE3, 0xE4, 0xEB, 0xEC, 0xED, 0xF4, 0xFC, 0xFD};

    std::filesystem::path opcodeFilePath(u8 opcode, bool isCbPrefixed)
    {
        std::filesystem::path dir(SM83_TEST_DATA_DIR);
        std::string fileName = isCbPrefixed
            ? std::format("cb {:02x}.json", opcode)
            : std::format("{:02x}.json", opcode);
        return dir / fileName;
    }

    void loadInitialState(CentralProcessingUnitJsonTestFixture& fixture, const json& initial)
    {
        fixture.setRegister(RegisterType::A, static_cast<u16>(initial.at("a").get<int>()));
        fixture.setRegister(RegisterType::B, static_cast<u16>(initial.at("b").get<int>()));
        fixture.setRegister(RegisterType::C, static_cast<u16>(initial.at("c").get<int>()));
        fixture.setRegister(RegisterType::D, static_cast<u16>(initial.at("d").get<int>()));
        fixture.setRegister(RegisterType::E, static_cast<u16>(initial.at("e").get<int>()));
        fixture.setRegister(RegisterType::F, static_cast<u16>(initial.at("f").get<int>()));
        fixture.setRegister(RegisterType::H, static_cast<u16>(initial.at("h").get<int>()));
        fixture.setRegister(RegisterType::L, static_cast<u16>(initial.at("l").get<int>()));
        fixture.setRegister(RegisterType::SP, static_cast<u16>(initial.at("sp").get<int>()));
        fixture.setRegister(RegisterType::PC, static_cast<u16>(initial.at("pc").get<int>()));
        fixture.setInterruptMasterEnabled(initial.at("ime").get<int>() != 0);
        if (initial.contains("ie"))
            fixture.setInterruptEnableRegister(static_cast<u8>(initial.at("ie").get<int>()));

        for (const auto& entry : initial.at("ram"))
            fixture.writeMemory(entry[0].get<u16>(), static_cast<u8>(entry[1].get<int>()));
    }

    void checkFinalState(const CentralProcessingUnitJsonTestFixture& fixture, const json& final)
    {
        CHECK(fixture.getRegister(RegisterType::A) == final.at("a").get<int>());
        CHECK(fixture.getRegister(RegisterType::B) == final.at("b").get<int>());
        CHECK(fixture.getRegister(RegisterType::C) == final.at("c").get<int>());
        CHECK(fixture.getRegister(RegisterType::D) == final.at("d").get<int>());
        CHECK(fixture.getRegister(RegisterType::E) == final.at("e").get<int>());
        CHECK(fixture.getRegister(RegisterType::F) == final.at("f").get<int>());
        CHECK(fixture.getRegister(RegisterType::H) == final.at("h").get<int>());
        CHECK(fixture.getRegister(RegisterType::L) == final.at("l").get<int>());
        CHECK(fixture.getRegister(RegisterType::SP) == final.at("sp").get<int>());
        CHECK(fixture.getRegister(RegisterType::PC) == final.at("pc").get<int>());
        CHECK(fixture.getInterruptMasterEnabled() == (final.at("ime").get<int>() != 0));

        for (const auto& entry : final.at("ram"))
            CHECK(fixture.readMemory(entry[0].get<u16>()) == static_cast<u8>(entry[1].get<int>()));
    }

    // Each JSON "cycles" entry is [address, value, tag]; tag's first
    // character is 'r' for a read, second character 'w' for a write, and
    // both '-' (e.g. "---") for an internal M-cycle with no bus access -
    // those don't correspond to an entry in our bus access log.
    void checkBusLog(const std::vector<FlatMemoryBus::BusAccess>& actual, const json& cycles)
    {
        std::vector<FlatMemoryBus::BusAccess> expected;
        for (const auto& cycle : cycles)
        {
            const std::string tag = cycle[2].get<std::string>();
            bool isRead = !tag.empty() && tag[0] == 'r';
            bool isWrite = tag.size() > 1 && tag[1] == 'w';
            if (!isRead && !isWrite)
                continue;

            expected.push_back({cycle[0].get<u16>(), static_cast<u8>(cycle[1].get<int>()), isWrite});
        }

        REQUIRE(actual.size() == expected.size());
        for (size_t i = 0; i < expected.size(); ++i)
        {
            CHECK(actual[i].address == expected[i].address);
            CHECK(actual[i].value == expected[i].value);
            CHECK(actual[i].isWrite == expected[i].isWrite);
        }
    }

    void runOpcodeTestFile(u8 opcode, bool isCbPrefixed)
    {
        std::filesystem::path path = opcodeFilePath(opcode, isCbPrefixed);
        std::ifstream file(path);
        REQUIRE_MESSAGE(file.is_open(), "Could not open ", path.string());

        json testCases;
        file >> testCases;

        size_t limit = std::min<size_t>(testCases.size(), GBR_JSON_CPU_TEST_CASE_LIMIT);
        for (size_t i = 0; i < limit; ++i)
        {
            const json& testCase = testCases[i];
            INFO("Test case: ", testCase.value("name", std::to_string(i)));

            CentralProcessingUnitJsonTestFixture fixture;
            loadInitialState(fixture, testCase.at("initial"));

            fixture.resetBusLog();
            fixture.step();

            // Snapshot before checkFinalState(), since its readMemory() calls
            // would otherwise also be recorded into the same bus access log.
            std::vector<FlatMemoryBus::BusAccess> busLog = fixture.busLog();

            checkFinalState(fixture, testCase.at("final"));
            checkBusLog(busLog, testCase.at("cycles"));
        }
    }
}

TEST_CASE("SM83 JSON CPU tests - unprefixed opcodes")
{
    for (int opcode = 0x00; opcode <= 0xFF; ++opcode)
    {
        if (SkippedUnprefixedOpcodes.count(static_cast<u8>(opcode)) > 0)
            continue;

        std::string subcaseName = std::format("opcode 0x{:02X}", opcode);
        SUBCASE(subcaseName.c_str())
        {
            runOpcodeTestFile(static_cast<u8>(opcode), false);
        }
    }
}

TEST_CASE("SM83 JSON CPU tests - CB-prefixed opcodes")
{
    for (int opcode = 0x00; opcode <= 0xFF; ++opcode)
    {
        std::string subcaseName = std::format("CB opcode 0x{:02X}", opcode);
        SUBCASE(subcaseName.c_str())
        {
            runOpcodeTestFile(static_cast<u8>(opcode), true);
        }
    }
}

#endif
