#pragma once
#include "hmain.hpp"

namespace PIN
{
    constexpr uint8 BOOST = 13;
    constexpr uint8 SCI = 18;
    constexpr uint8 SDI = 15;
    constexpr uint8 SII = 12;
    constexpr uint8 SDO = 14;
    constexpr uint8 POWER = 16;
}

constexpr auto TINY_SPI = spi1;

inline uint8_t usbmemory[1024];
inline uint8_t poll[2];