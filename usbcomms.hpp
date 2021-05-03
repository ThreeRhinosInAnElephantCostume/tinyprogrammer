#pragma once
#include "hmain.hpp"
enum class USB_RESPONSES
{
    RESERVED = 0, // in effect, the first byte of the return message will only be zero if something goes wrong
    OK = 1,
    INVALID_COMMAND = 2,
    FAILURE=3,
    NOTREADY = 4,
    NOTPOWERED = 5,
    NOTCHECKED = 6,
    CHIPFAULT = 7,
};
enum class CMD
{
    ECHO=0,

    POWER_ON,
    POWER_OFF,

    CHECK,

    READ_DATA,
    WRITE_DATA,

    READ_FLASH, 
    WRITE_FLASH,

    READ_EEPROM,
    WRITE_EEPROM,

    READ_FUSES,
    WRITE_FUSES,

    READ_HASH_DATA,
    POWERED,
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
    uint8_t rcmd;
};
struct __attribute__((__packed__)) Echo : Command
{
    uint8_t len;
    const char text[62];
};
void usb_task();

inline std::unique_ptr<HVP> hvp = nullptr; 
inline std::unique_ptr<ChipDesc> chip_desc = nullptr;
