# gb-emulator

A Game Boy and Game Boy Color emulator written in C++20 with SDL2. Builds on Linux and Windows.

## Status

- [x] Project skeleton: CMake, SDL2 window, ROM loading and header parsing
- [ ] Stage 1: memory bus, SM83 CPU, timer, interrupts (target: Blargg `cpu_instrs`)
- [ ] Stage 2: PPU, input (target: `dmg-acid2`, Tetris)
- [ ] Stage 3: MBC1/MBC3/MBC5, battery saves, RTC, APU sound (target: Pokémon Red with sound)
- [ ] Stage 4: Game Boy Color mode (target: `cgb-acid2`, Pokémon Crystal)

## Building

### Linux (Debian, Ubuntu, Mint)

```sh
sudo apt install build-essential cmake libsdl2-dev
cmake -S . -B build
cmake --build build -j
./build/gbemu path/to/game.gb
```

### Windows

Install Visual Studio 2022 (workload "Desktop development with C++") and CMake. SDL2 is downloaded automatically during configuration.

```bat
cmake -S . -B build
cmake --build build --config Release
build\Release\gbemu.exe path\to\game.gb
```

### Tests

```sh
ctest --test-dir build --output-on-failure
```

## Layout

```
src/core/      emulator core, no platform dependencies
src/frontend/  SDL2 window, input and audio
tests/         unit tests
```

ROM files are not included. Use your own legally obtained ROMs or public test ROMs.
