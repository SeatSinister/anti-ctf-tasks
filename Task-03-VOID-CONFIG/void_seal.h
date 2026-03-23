#pragma once
#include <cstdint>

// Фрагмент «северного» контура телеметрии (не совпадает с эталоном из комментария в main.cpp).
namespace vs {
constexpr uint32_t kTelemetrySaltA = 0x12345678u;

// FNV-1a по упакованному кортежу состояния UI (модуль, FOV little-endian, индекс режима).
inline uint32_t TmPacketHash(bool module_on, int fov_deg, int mode_idx) {
    uint32_t h = 0x811C9DC5u;
    auto feed = [&](uint8_t b) { h = ((h ^ b) * 0x01000193u); };
    feed(module_on ? 1u : 0u);
    feed(static_cast<uint8_t>(fov_deg & 0xFF));
    feed(static_cast<uint8_t>((fov_deg >> 8) & 0xFF));
    feed(static_cast<uint8_t>(mode_idx & 0xFF));
    return h;
}
} // namespace vs
