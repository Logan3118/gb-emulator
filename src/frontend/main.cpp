#include "core/cartridge.h"

#include <SDL.h>

#include <array>
#include <cstdint>
#include <cstdio>
#include <string>

namespace {

constexpr int kScreenWidth = 160;
constexpr int kScreenHeight = 144;
constexpr int kDefaultScale = 4;

void printHeader(const gb::Cartridge& cart) {
    const auto& h = cart.header();
    std::printf("Title:     %s\n", h.title.c_str());
    std::printf("Type:      %s (0x%02X)\n", h.cartridgeTypeName().c_str(), h.cartridgeType);
    std::printf("ROM size:  %zu KiB\n", h.romSizeBytes() / 1024);
    std::printf("RAM size:  %zu KiB\n", h.ramSizeBytes() / 1024);
    std::printf("CGB:       %s\n", h.cgbOnly() ? "CGB only" : (h.supportsCgb() ? "CGB enhanced" : "no"));
    std::printf("Checksum:  %s\n", cart.headerChecksumValid() ? "OK" : "BAD");
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::fprintf(stderr, "Usage: %s <rom.gb|rom.gbc>\n", argv[0]);
        return 1;
    }

    std::string error;
    auto cart = gb::Cartridge::fromFile(argv[1], &error);
    if (!cart) {
        std::fprintf(stderr, "Failed to load ROM: %s\n", error.c_str());
        return 1;
    }
    printHeader(*cart);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    const std::string windowTitle = "gbemu - " + cart->header().title;
    SDL_Window* window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          kScreenWidth * kDefaultScale, kScreenHeight * kDefaultScale,
                                          SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer =
        window ? SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC) : nullptr;
    if (window && !renderer) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE); // e.g. no GPU driver
    }
    SDL_Texture* texture = renderer ? SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                                        SDL_TEXTUREACCESS_STREAMING, kScreenWidth, kScreenHeight)
                                    : nullptr;
    if (!texture) {
        std::fprintf(stderr, "SDL setup failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_RenderSetLogicalSize(renderer, kScreenWidth, kScreenHeight);

    // Placeholder framebuffer until the PPU exists: the classic DMG "off" green.
    std::array<uint32_t, kScreenWidth * kScreenHeight> framebuffer{};
    framebuffer.fill(0xFF9BBC0F);

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT ||
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                running = false;
            }
        }

        SDL_UpdateTexture(texture, nullptr, framebuffer.data(), kScreenWidth * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
