#include "core/cartridge.h"

#include <fstream>
#include <iterator>

namespace gb {

namespace {

constexpr std::size_t kHeaderEnd = 0x0150;
constexpr std::size_t kTitleStart = 0x0134;
constexpr std::size_t kTitleMaxLen = 16;

void setError(std::string* error, std::string message) {
    if (error) {
        *error = std::move(message);
    }
}

} // namespace

std::size_t CartridgeHeader::romSizeBytes() const {
    if (romSizeCode <= 0x08) {
        return std::size_t{32 * 1024} << romSizeCode;
    }
    return 0;
}

std::size_t CartridgeHeader::ramSizeBytes() const {
    switch (ramSizeCode) {
    case 0x02: return 8 * 1024;
    case 0x03: return 32 * 1024;
    case 0x04: return 128 * 1024;
    case 0x05: return 64 * 1024;
    default: return 0;
    }
}

std::string CartridgeHeader::cartridgeTypeName() const {
    switch (cartridgeType) {
    case 0x00: return "ROM ONLY";
    case 0x01: return "MBC1";
    case 0x02: return "MBC1+RAM";
    case 0x03: return "MBC1+RAM+BATTERY";
    case 0x05: return "MBC2";
    case 0x06: return "MBC2+BATTERY";
    case 0x0F: return "MBC3+TIMER+BATTERY";
    case 0x10: return "MBC3+TIMER+RAM+BATTERY";
    case 0x11: return "MBC3";
    case 0x12: return "MBC3+RAM";
    case 0x13: return "MBC3+RAM+BATTERY";
    case 0x19: return "MBC5";
    case 0x1A: return "MBC5+RAM";
    case 0x1B: return "MBC5+RAM+BATTERY";
    case 0x1C: return "MBC5+RUMBLE";
    case 0x1D: return "MBC5+RUMBLE+RAM";
    case 0x1E: return "MBC5+RUMBLE+RAM+BATTERY";
    default: return "UNKNOWN";
    }
}

uint8_t computeHeaderChecksum(std::span<const uint8_t> rom) {
    uint8_t sum = 0;
    for (std::size_t addr = 0x0134; addr <= 0x014C; ++addr) {
        sum = static_cast<uint8_t>(sum - rom[addr] - 1);
    }
    return sum;
}

Cartridge::Cartridge(std::vector<uint8_t> rom, CartridgeHeader header)
    : rom_(std::move(rom)), header_(std::move(header)) {}

std::optional<Cartridge> Cartridge::fromBytes(std::vector<uint8_t> rom, std::string* error) {
    if (rom.size() < kHeaderEnd) {
        setError(error, "file is too small to be a Game Boy ROM");
        return std::nullopt;
    }

    CartridgeHeader h;
    h.cgbFlag = rom[0x0143];
    h.cartridgeType = rom[0x0147];
    h.romSizeCode = rom[0x0148];
    h.ramSizeCode = rom[0x0149];
    h.headerChecksum = rom[0x014D];

    // CGB games reuse the last title bytes for the manufacturer code and CGB flag.
    const std::size_t titleLen = h.supportsCgb() ? 15 : kTitleMaxLen;
    for (std::size_t i = 0; i < titleLen; ++i) {
        const char c = static_cast<char>(rom[kTitleStart + i]);
        if (c == '\0') {
            break;
        }
        h.title.push_back(c);
    }

    return Cartridge(std::move(rom), std::move(h));
}

std::optional<Cartridge> Cartridge::fromFile(const std::filesystem::path& path, std::string* error) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        setError(error, "cannot open " + path.string());
        return std::nullopt;
    }
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return fromBytes(std::move(data), error);
}

bool Cartridge::headerChecksumValid() const {
    return computeHeaderChecksum(rom_) == header_.headerChecksum;
}

} // namespace gb
