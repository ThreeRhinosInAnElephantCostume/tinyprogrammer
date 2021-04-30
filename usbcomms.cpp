#include "hmain.hpp"
enum class USB_RESPONSES
{
    OK = 0,
    INVALID_COMMAND = 1,
};
enum class CMD
{
    ECHO,

    CHECK,

    WRITE_DATA,
    READ_DATA,

    READ_FLASH, 
    WRITE_FLASH,

    READ_EEPROM,
    WRITE_EEPROM,

    READ_FUSES,
    WRITE_FUSES,
};
struct __attribute__((__packed__)) response
{
    uint8_t errcode = 0;
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
    tud_cdc_write((void*)&res, sizeof(res));
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
    res.errcode = 0;
    res.len = dt->len;
    memcpy(res.data, dt->text, dt->len);
    return res;
}
void usb_task()
{
    if(!tud_cdc_available())
        return;
    uint8_t data[64];
    tud_cdc_read(&data, 64);
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