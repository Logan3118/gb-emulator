#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace gb {

// Parsed cartridge header (ROM bytes 0x0100-0x014F).
struct CartridgeHeader {
    std::string title;
    uint8_t cgbFlag = 0;        // 0x0143: 0x80 = CGB enhanced, 0xC0 = CGB only
    uint8_t cartridgeType = 0;  // 0x0147: mapper and extra hardware
    uint8_t romSizeCode = 0;    // 0x0148
    uint8_t ramSizeCode = 0;    // 0x0149
    uint8_t headerChecksum = 0; // 0x014D

    bool supportsCgb() const { return (cgbFlag & 0x80) != 0; }
    bool cgbOnly() const { return cgbFlag == 0xC0; }
    std::size_t romSizeBytes() const;
    std::size_t ramSizeBytes() const;
    std::string cartridgeTypeName() const;
};

class Cartridge {
public:
    // Returns std::nullopt and fills `error` if the data is not a valid ROM.
    static std::optional<Cartridge> fromBytes(std::vector<uint8_t> rom, std::string* error = nullptr);
    static std::optional<Cartridge> fromFile(const std::filesystem::path& path, std::string* error = nullptr);

    const CartridgeHeader& header() const { return header_; }
    std::span<const uint8_t> rom() const { return rom_; }
    bool headerChecksumValid() const;

private:
    Cartridge(std::vector<uint8_t> rom, CartridgeHeader header);

    std::vector<uint8_t> rom_;
    CartridgeHeader header_;
};

// Computes the header checksum over bytes 0x0134-0x014C, as the boot ROM does.
uint8_t computeHeaderChecksum(std::span<const uint8_t> rom);

} // namespace gb
