    // This file is part of tinyavrprogrammer.

    // tinyavrprogrammer is free software: you can redistribute it and/or modify
    // it under the terms of the GNU General Public License as published by
    // the Free Software Foundation, version 3.

    // tinyavrprogrammer is distributed in the hope that it will be useful,
    // but WITHOUT ANY WARRANTY; without even the implied warranty of
    // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    // GNU General Public License for more details.

    // You should have received a copy of the GNU General Public License
    // along with tinyavrprogrammer.  If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include "hmain.hpp"
enum class USB_RESPONSES
{
    RESERVED = 0, // in effect, the first byte of the return message will only be zero if something goes wrong
    OK = 1,
    // cmd errors
    INVALID_COMMAND,
    INVALID_ARGUMENT,
    INVALID_RANGE, // out of range, etc
    // hardware errors
    FAILURE,
    CHIPFAULT,
    // logic errors
    NOTREADY,
    NOTPOWERED,
    NOTCHECKED,
    NOTERASED
};
enum class CMD
{
    ECHO=0,

    PROG_READY,
    CHIP_POWERED,

    POWER_ON,
    POWER_OFF,

    CHECK,
    
    CHIP_ERASE,

    READ_DATA,
    WRITE_DATA,
    READ_HASH_DATA,

    READ_FLASH, 
    WRITE_FLASH,

    READ_EEPROM,
    WRITE_EEPROM,

    READ_FUSES,
    WRITE_FUSES,

    WRITE_LOCK,

    READ_CALIBRATION,

    WAS_ERASED
};
struct __attribute__((__packed__)) ChipDesc 
{
    union
    {
        struct
        {
            bool lock1 : 1;
            bool lock2 : 1;
        };
        uint8 lock;
    };

    uint8 fuseex;
    uint8 fusehigh;
    uint8 fuselow;

    uint8 signature[3];
    uint8 calibration;

    CHIPS::ChipInfo info;
};
struct __attribute__((__packed__)) Response
{
    uint8_t errcode = (uint8_t)USB_RESPONSES::OK;
    uint8_t len = 0;
    uint8_t data[62] = {0};
    Response()
    {

    }
    Response(USB_RESPONSES errcode, uint8_t len = 0, void* data = nullptr)
    {
        this->errcode = (uint8)errcode;
        this->len = len;
        if(data != nullptr && len > 0)
        {
            memcpy(this->data, data, len);
        }
    }
};
struct __attribute__((__packed__)) Command
{
    uint8 rcmd;
};
struct __attribute__((__packed__)) Echo : Command
{
    uint8 len;
    const char text[62];
};
struct __attribute__((__packed__)) ReadData : Command // up to 60 bytes at a time
{
    uint16 address;
    uint8 len;
};
struct __attribute__((__packed__)) WriteData : Command // up to 60 bytes at a time
{
    uint16 address;
    uint8 len;
    uint8 data[60];
};
struct __attribute__((__packed__)) HashData : Command
{
    uint16 address;
    uint16 len;
};
struct __attribute__((__packed__)) RWPaged : Command 
{
    uint16 startpage;
    uint16 npages;
    uint16 memaddr;
};
struct __attribute__((__packed__)) WriteFuses : Command 
{
    uint8 low;
    uint8 high;
    uint8 extended;
};
struct __attribute__((__packed__)) WriteLock : Command 
{
    uint8 lock;
};

inline std::shared_ptr<HVP> hvp = nullptr; 
inline std::shared_ptr<ChipDesc> chip_desc = nullptr;

inline bool chip_erased = false;

bool usb_task();
void power_off();