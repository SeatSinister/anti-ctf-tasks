#pragma once
#include <cstdint>

// Второй файл — остаток константы эталона (сборка только на стороне клиента).
namespace vsf {
constexpr uint32_t kTelemetrySaltB = 0xABCDEF01u;
constexpr uint32_t kTelemetrySaltC = 0x1AB34D3Du;
} // namespace vsf
