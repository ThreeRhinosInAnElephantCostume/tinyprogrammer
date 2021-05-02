#pragma once
#include "hmain.hpp"

#define DEBUG 1


namespace PIN
{
    constexpr uint8 SCI = 18;
    constexpr uint8 SDI = 15;
    constexpr uint8 SII = 14;
    constexpr uint8 SDO = 12;
    constexpr uint8 POWER = 16;
    constexpr uint8 PULSE = 13;
    constexpr uint8 HIGHVOLT = 18;
}
namespace ADC
{
    constexpr uint8 BOOSTREAD = 0;
    constexpr float BOOSTREAD_MP = 10.0f/0.98912f;
    constexpr uint8 VBUSREAD = 1;
    constexpr float VBUSREAD_MP = 2.5f;
}
namespace BOOST
{
    constexpr uint8 DIVIDER = 1;
    constexpr uint16 WRAP = 512;
    constexpr uint16 MAX_DUTY = WRAP*0.8;
    constexpr uint16 MIN_DUTY = 0;
    constexpr uint16 DEFAULT_DUTY = MIN_DUTY;
    constexpr uint16 MINCHANGE = 1;
    constexpr uint16 MAXCHANGE = 5;
    constexpr float CHANGEBASE = 1.0f; // max(MINCHANGE, min(MAXCHANGE, TARGET-V * CHANGEBASE ))

    constexpr float TARGET = 12.0f;
    constexpr float MAX_SAFE = TARGET + 0.3f;
    constexpr float MIN_SAFE = TARGET - 0.3f;
    constexpr float OVERVOLT = TARGET + 0.45f;
    constexpr float UNDERVOLT = TARGET - 0.45f;

    constexpr float VBUS_MAXIMUM = 5.4f;
    constexpr float VBUS_MINIMUM = 4.6f;
}
namespace MULTICORE
{
    constexpr uint32 FAILURE_CODE = 666;
}

inline uint8_t usbmemory[1 << 16];


inline bool is_power_safe = false;
inline float boostvoltage = 0.0f;

inline bool chippowered = false;

inline bool panicnow = false;