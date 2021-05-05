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
struct __attribute__((__packed__)) HashData : Command // up to 60 bytes at a time
{
    uint16 address;
    uint8 len;
};
struct __attribute__((__packed__)) ReadFlash : Command // up to 60 bytes at a time
{
    uint16 startpage;
    uint16 npages;
    uint16 destination;
};

void usb_task();

inline std::shared_ptr<HVP> hvp = nullptr; 
inline std::shared_ptr<ChipDesc> chip_desc = nullptr;

inline bool chip_erased = false;