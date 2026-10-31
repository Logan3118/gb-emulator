#include "core/cartridge.h"
#include "test.h"

#include <cstring>

namespace {

std::vector<uint8_t> makeRom(const char* title, uint8_t cgbFlag, uint8_t type, uint8_t romCode, uint8_t ramCode) {
    std::vector<uint8_t> rom(32 * 1024, 0);
    std::memcpy(&rom[0x0134], title, std::strlen(title));
    rom[0x0143] = cgbFlag;
    rom[0x0147] = type;
    rom[0x0148] = romCode;
    rom[0x0149] = ramCode;
    rom[0x014D] = gb::computeHeaderChecksum(rom);
    return rom;
}

} // namespace

TEST(rejects_too_small_file) {
    std::string error;
    auto cart = gb::Cartridge::fromBytes(std::vector<uint8_t>(0x100, 0), &error);
    CHECK(!cart.has_value());
    CHECK(!error.empty());
}

TEST(parses_dmg_header) {
    auto cart = gb::Cartridge::fromBytes(makeRom("POKEMON RED", 0x00, 0x13, 0x05, 0x03));
    CHECK(cart.has_value());
    const auto& h = cart->header();
    CHECK(h.title == "POKEMON RED");
    CHECK(!h.supportsCgb());
    CHECK(h.cartridgeTypeName() == "MBC3+RAM+BATTERY");
    CHECK(h.romSizeBytes() == 1024 * 1024);
    CHECK(h.ramSizeBytes() == 32 * 1024);
    CHECK(cart->headerChecksumValid());
}

TEST(parses_cgb_header) {
    auto cart = gb::Cartridge::fromBytes(makeRom("PM_CRYSTAL", 0x80, 0x10, 0x06, 0x03));
    CHECK(cart.has_value());
    CHECK(cart->header().supportsCgb());
    CHECK(!cart->header().cgbOnly());
    CHECK(cart->header().title == "PM_CRYSTAL");
}

TEST(detects_bad_checksum) {
    auto rom = makeRom("TETRIS", 0x00, 0x00, 0x00, 0x00);
    rom[0x014D] ^= 0xFF;
    auto cart = gb::Cartridge::fromBytes(std::move(rom));
    CHECK(cart.has_value());
    CHECK(!cart->headerChecksumValid());
}
