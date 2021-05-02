#include "hmain.hpp"
enum class USB_RESPONSES
{
    RESERVED = 0, // in effect, the first byte of the return message will only be zero if something goes wrong
    OK = 1,
    INVALID_COMMAND = 2,
};
enum class CMD
{
    ECHO=0,

    POWER_ON,
    POWER_OFF,

    CHECK,

    WRITE_DATA,
    READ_DATA,

    READ_FLASH, 
    WRITE_FLASH,

    READ_EEPROM,
    WRITE_EEPROM,

    READ_FUSES,
    WRITE_FUSES,

    READ_HASH_DATA,
};
struct __attribute__((__packed__)) response
{
    uint8_t errcode = (uint8_t)USB_RESPONSES::OK;
    uint8_t len = 0;
    uint8_t data[62] = {0};
};
struct __attribute__((__packed__)) command
{
    uint8_t rcmd;
};
struct __attribute__((__packed__)) echo : command
{
    uint8_t len;
    const char text[62];
};
void usb_TX(response res)
{
    tud_vendor_write((void*)&res, sizeof(res));
}
void usb_TX(uint8_t errcode)
{
    response res;
    res.errcode = errcode;
    res.len = 0;
    usb_TX(res);
}
template<typename T>
void __create_response(response* res, T val)
{
    if(res->len+sizeof(val) > sizeof(res->data))
    {
        gracefulfailure();
    }
    memcpy(res->data+res->len, &val, sizeof(val));
    res->len += sizeof(val);
}
template<typename T, typename... Ts>
void __create_response(response* res, T first, Ts... rest)
{
    __create_response(res, first);
    __create_response(res, rest...);
}
template<typename T, typename... Ts>
response create_response(USB_RESPONSES errcode=USB_RESPONSES::OK, Ts... data)
{
    response res;
    res.errcode = errcode;
    res.len = 0;
    __create_response(&res, data...);
    return res;
}
response cmd_echo(void* data)
{
    echo* dt = (echo*)data;
    response res;
    res.errcode = (uint8_t)USB_RESPONSES::OK;
    res.len = dt->len;
    memcpy(res.data, dt->text, dt->len);
    return res;
}
void usb_task()
{
    if(!tud_vendor_available())
        return;
    uint8_t data[64];
    tud_vendor_read(&data, 64);
    command* rcmd = (command*)data;
    std::vector<std::function<response(void* data)>> fs = 
    {
        cmd_echo
    };
    response res;
    if(rcmd->rcmd >= fs.size())
    {
        res.errcode = (uint8_t) USB_RESPONSES::INVALID_COMMAND;
        res.len = 0;
    }
    else
    {
        res = fs[rcmd->rcmd](data);
    }
    usb_TX(res);
}