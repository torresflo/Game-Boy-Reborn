![GitHub license](https://img.shields.io/github/license/torresflo/Game-Boy-Reborn.svg)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](http://makeapullrequest.com)
![GitHub contributors](https://img.shields.io/github/contributors/torresflo/Game-Boy-Reborn.svg)
![GitHub issues](https://img.shields.io/github/issues/torresflo/Game-Boy-Reborn.svg)

<p align="center">
  <h1 align="center">Game Boy Reborn</h3>

  <p align="center">
    A cycle-accurate Nintendo Game Boy (DMG-01) emulator written in modern C++20, created just for fun!
    <br/>
    <strong>Windows only.</strong>
    <br/>
    <a href="https://github.com/torresflo/Game-Boy-Reborn/issues">Report a bug or request a feature</a>
  </p>
</p>

## Table of Contents

* [Overview](#overview)
* [ROM Compatibility](#rom-compatibility)
* [For Players](#for-players)
  * [Controls](#controls)
  * [Save States & Battery Saves](#save-states--battery-saves)
  * [Getting ROMs](#getting-roms)
* [For Developers](#for-developers)
  * [Prerequisites](#prerequisites)
  * [Building](#building)
  * [Running Tests](#running-tests)
  * [Project Layout](#project-layout)
  * [Debugging tools](#debugging-tools)
* [Contributing](#contributing)
* [License](#license)
* [References](#references)

## Overview

Game Boy Reborn aims to emulate the original DMG-01 Game Boy as faithfully as possible, down to per-cycle CPU and PPU timing, rather than just running games "well enough".

Current highlights:

- **CPU**: full SM83 instruction set, verified against the [SingleStepTests/sm83](https://github.com/SingleStepTests/sm83) per-opcode test vectors.
- **PPU**: background, window and sprite (OBJ) rendering to a 160x144 framebuffer.
- **APU**: all 4 original sound channels (2 pulse, wave, noise) with the frame sequencer and mixer, resampled to 44100 Hz stereo output.
- **Save states**: quick save/load (F5/F9) plus save to file/load from file dialogs.
- **Battery-backed cartridge RAM**: saved next to the ROM as a `.sav` file.
- **Input**: keyboard and gamepad (Xbox-style layout) support.
- **Adjustable emulation speed**: from 0.125x up to 8x.
- **Built-in debugging tools**: CPU register viewer, cartridge info, tile data viewer, object (sprite) viewer, a log viewer, a disassembly viewer, and a full memory hex viewer (0x0000–0xFFFF).

## ROM Compatibility

The emulator targets original DMG cartridges. Game Boy Color-only titles are not supported, but Game Boy / Super Game Boy / GBC dual-mode cartridges run in their DMG mode.

Compatibility depends on the cartridge's Memory Bank Controller (MBC), declared in its header:

| Cartridge type (header byte)  | Mapper                                    | Status            |
|-------------------------------|-------------------------------------------|-------------------|
| `0x00`                        | ROM ONLY                                  | ✅ Supported      |
| `0x01`–`0x03`                 | MBC1 (+RAM, +Battery)                     | ✅ Supported      |
| `0x05`–`0x06`                 | MBC2 (+Battery)                           | ✅ Supported      |
| `0x08`–`0x09`                 | ROM+RAM (+Battery)                        | ✅ Supported      |
| `0x0F`–`0x13`                 | MBC3 (+Timer, +RAM, +Battery)             | ✅ Supported      |
| `0x19`–`0x1E`                 | MBC5 (+RAM, +Rumble, +Battery)            | ✅ Supported      |
| `0x0B`–`0x0D`                 | MMM01 (+RAM, +Battery)                    | ❌ Not supported  |
| `0x20`                        | MBC6                                      | ❌ Not supported  |
| `0x22`                        | MBC7 (+Sensor, +Rumble, +RAM, +Battery)   | ❌ Not supported  |
| `0xFE`–`0xFF`                 | HuC3 / HuC1                               | ❌ Not supported  |

The detected cartridge type, ROM/RAM size and licensee are shown in the in-app **Debug > Cartridge Info** window, which is the quickest way to check why a given ROM refuses to load.

## For Players

### Controls

| Game Boy  | Keyboard      | Gamepad (Xbox layout) |
|-----------|---------------|-----------------------|
| D-Pad     | Arrow keys    | Left stick / D-Pad    |
| A         | Enter         | Button A              |
| B         | Backspace     | Button B              |
| Start     | Escape        | Start                 |
| Select    | Tab           | Back/Select           |

Other shortcuts:

| Key                   | Action                                    |
|-----------------------|-------------------------------------------|
| `F1`                  | Show/hide the menu bar                    |
| `F11`                 | Toggle fullscreen                         |
| `Pause`               | Pause/resume emulation                    |
| `+` / `-` (numpad)    | Cycle through speed presets (0.25x to 8x) |
| `=`                   | Reset speed to 1x                         |
| `F5`                  | Quick-save state                          |
| `F9`                  | Quick-load state                          |

### Save States & Battery Saves

- **Save states** snapshot the entire emulator (CPU, memory, PPU, APU, cartridge) so you can resume later. Use `F5`/`F9` for a quick slot, or `File > Save State As...` / `File > Load State...` to pick a file.
- **Battery saves**: for cartridges with battery-backed RAM (most games that save progress on real hardware), the RAM is automatically written to a `.sav` file next to the ROM when the ROM changes or the emulator closes, and reloaded the next time you open that ROM.

### Getting ROMs

Game Boy Reborn does not ship with, host, or link to any ROM files. To use the emulator you need your own legally-obtained ROM dumps (for example, from cartridges you own). The `roms/` folder in this repository is for local testing only.

## For Developers

### Prerequisites

- **Windows** (the only platform currently supported, via MSVC)
- **CMake 3.28+**
- **MSVC** (Visual Studio 2022 or later, with the "Desktop development with C++" workload)

All other dependencies are fetched automatically by CMake's `FetchContent`, there is nothing to install manually:

| Dependency                                                                                        | Purpose                                                                       |
|---------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------|
| [SFML 3.1.0](https://www.sfml-dev.org/)                                                           | Window, input and audio output (Network module disabled, unused)              |
| [Dear ImGui](https://github.com/ocornut/imgui) + [imgui-sfml](https://github.com/SFML/imgui-sfml) | Debug UI widgets                                                              |
| [ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog)                                     | ROM / save-state file pickers                                                 |
| [imgui_club](https://github.com/ocornut/imgui_club) (`imgui_memory_editor`)                       | Hex viewer widget for the Memory Viewer debug tool                            |
| [doctest](https://github.com/doctest/doctest) + [nlohmann/json](https://github.com/nlohmann/json) | Unit test framework (test target only)                                        |
| [SingleStepTests/sm83](https://github.com/SingleStepTests/sm83)                                   | External per-opcode CPU test vectors (test target only, build directory only) |

### Building

```bash
git clone https://github.com/torresflo/Game-Boy-Reborn.git
cd Game-Boy-Reborn
cmake -B build
cmake --build build --config Release
```

The first configure step will download all dependencies above, which can take a while. The resulting executable is `Game-Boy-Reborn` (built from `src/main.cpp`). The emulation logic and UI also build as the `GameBoyCore` and `GameBoyUI` static libraries respectively.

Useful CMake options (pass with `-D<option>=<value>` at configure time):

| Option                            | Default   | Description                                                   |
|-----------------------------------|-----------|---------------------------------------------------------------|
| `GBR_ENABLE_JSON_CPU_TESTS`       | `ON`      | Fetch and run the SingleStepTests/sm83 CPU test vectors       |
| `GBR_JSON_CPU_TEST_CASE_LIMIT`    | `100`     | Max test cases run per opcode JSON file (each file has 1000)  |

### Running Tests

Building the project also builds the `Game-Boy-Reborn-Tests` executable. Run the full suite via CTest from the build directory:

```bash
ctest --test-dir build
```

### Project Layout

```
src/
  Core/   GameBoyCore   > emulation only (CPU, PPU, APU, memory bus, cartridge/MBCs)
  UI/     GameBoyUI     > SFML window, input and Dear ImGui debug widgets, links GameBoyCore
  main.cpp              > entry point, constructs the Application run it
tests/                  > unit tests linked against GameBoyCore only
docs/                   > hardware reference docs
```

Each layer follows a consistent pattern: emulator-side state is exposed through small `const` accessors, and the UI layer reads them to draw widgets without owning emulation state itself.

### Debugging tools

All debug windows are accessible from the **Debug** menu:

| Window            | Description                                                               |
|-------------------|---------------------------------------------------------------------------|
| Cartridge Info    | ROM/RAM size, MBC type and licensee                                       |
| CPU Registers     | Live view of all CPU registers, flags, IME, halt state, and cycle count   |
| Disassembly       | Scrollable list of upcoming instructions starting at the current PC       |
| Tile Data         | Visual grid of all tiles currently in VRAM                                |
| Objects (sprites) | State of the 40 OAM sprite entries                                        |
| Memory Viewer     | Hex viewer for the full 0x0000–0xFFFF address space, updated every frame  |
| Log               | Scrollable log output with level filtering                                |

## Contributing

Contributions are what make the open source community such an amazing place to be, learn, inspire, and create. Any contributions you make are **greatly appreciated**.

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

Distributed under the GNU General Public License v3.0. See [LICENSE](LICENSE) for more information.

## References

Hardware references used while building this emulator:

- [Pan Docs](https://gbdev.io/pandocs/), a comprehensive Game Boy hardware reference
- [Game Boy CPU opcode table](https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html)
- [Official Nintendo Game Boy Programming Manual](https://archive.org/details/GameBoyProgManVer1.1/)
- [The Cycle-Accurate Game Boy Docs](https://github.com/rockytriton/LLD_gbemu/raw/main/docs/The%20Cycle-Accurate%20Game%20Boy%20Docs.pdf)
- [Game Boy CPU Technical Reference (gbctr)](https://github.com/rockytriton/LLD_gbemu/raw/main/docs/gbctr.pdf)
- [SingleStepTests/sm83](https://github.com/SingleStepTests/sm83), per-opcode CPU test vectors used by the test suite
