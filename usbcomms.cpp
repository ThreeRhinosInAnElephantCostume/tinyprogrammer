#include "hmain.hpp"
#include "usbcomms.hpp"
#define returnifnotsetup() if(!chippowered || !chip_desc) {return Response((!chippowered)? USB_RESPONSES::NOTPOWERED : USB_RESPONSES::NOTCHECKED);};

void usb_TX(Response res)
{
    tud_vendor_write((void*)&res, sizeof(res));
}
void usb_TX(uint8_t errcode)
{
    Response res;
    res.errcode = errcode;
    res.len = 0;
    usb_TX(res);
}
template<typename T>
void __create_response(Response* res, T val)
{
    if(res->len+sizeof(val) > sizeof(res->data))
    {
        gracefulfailure();
    }
    memcpy(res->data+res->len, &val, sizeof(val));
    res->len += sizeof(val);
}
template<typename T, typename... Ts>
void __create_response(Response* res, T first, Ts... rest)
{
    __create_response(res, first);
    __create_response(res, rest...);
}
template<typename T, typename... Ts>
Response create_response(USB_RESPONSES errcode=USB_RESPONSES::OK, Ts... data)
{
    Response res;
    res.errcode = errcode;
    res.len = 0;
    __create_response(&res, data...);
    return res;
}
Response cmd_echo(void* data)
{
    Echo* dt = (Echo*)data;
    Response res;
    res.errcode = (uint8_t)USB_RESPONSES::OK;
    res.len = dt->len;
    memcpy(res.data, dt->text, dt->len);
    return res;
}
Response cmd_poweron(void* data)
{
    hvp = nullptr;
    if(!is_power_safe)
    {
        return Response{USB_RESPONSES::NOTREADY};
    }
    init_out({PIN::SDI, PIN::SII, PIN::SDO, PIN::SCI}, false);
    init_out({PIN::POWER, PIN::HIGHVOLT}, false);
    sleep_us(10);
    gpio_put(PIN::POWER, true);
    sleep_us(20);
    gpio_put(PIN::HIGHVOLT, true);
    sleep_us(10);
    init_in(PIN::SDO);
    sleep_us(300);
    hvp = std::make_unique<HVP>(progpio, PIN::SDI, PIN::SII, PIN::SDO, PIN::SCI);
    chippowered = true;
    return Response(USB_RESPONSES::OK);
}
Response cmd_poweroff(void* data)
{
    if(!chippowered)
    {
        return Response(USB_RESPONSES::OK);
    }
    hvp = nullptr;
    init_out({PIN::SDI, PIN::SII, PIN::SCI}, false);
    gpio_put(PIN::SCI, 0);
    gpio_put(PIN::SII, 0);
    gpio_put(PIN::SDI, 0);
    gpio_put(PIN::HIGHVOLT, 0);
    gpio_put(PIN::POWER, 0);
    chippowered = false;
    return Response(USB_RESPONSES::OK);
}
Response cmd_check(void* data)
{
    if(!chippowered)
    {
        return Response(USB_RESPONSES::NOTPOWERED);
    }
    chip_desc = std::make_unique<ChipDesc>();

    // lock
    hvp->TX_RX(0b00001000, 0b01001100);
    hvp->TX_RX(0b00000000, 0b01111000);
    uint16 bits = hvp->TX_RX(0b00000000, 0b01111000);
    chip_desc->lock1 = !!(bits & (1 << 7));
    chip_desc->lock2 = !!(bits & (1 << 6));
    
    // signature
    hvp->TX_RX(0b00001000, 0b01001100);
    for(uint8 i = 0; i < 3; i++)
    {
        hvp->TX_RX(i << 6, 0b00001100);
        if(i == 0)
        {
            hvp->TX_RX(0b0, 0b01101000);
        }
        uint16 v = hvp->TX_RX(0b0, 0b01101100);
        chip_desc->signature[i] = v & 0xFF;
    }

    // calibration
    hvp->TX_RX(0b00001000, 0b01001100);
    hvp->TX_RX(0b0, 0b00001100);
    hvp->TX_RX(0b0, 0b01111000);
    chip_desc->calibration = hvp->TX_RX(0b0, 0b01111100) & 0xFF;

    // fuses
    hvp->TX_RX(0b00000100, 0b01001100);
    hvp->TX_RX(0b0, 0b01101000);
    chip_desc->fuselow = hvp->TX_RX(0b0, 0b01111110) & 0xFF;

    hvp->TX_RX(0b00000100, 0b01001100);
    hvp->TX_RX(0b0, 0b01111010);
    chip_desc->fusehigh = hvp->TX_RX(0b0, 0b01101110) & 0xFF;

    hvp->TX_RX(0b00000100, 0b01001100);
    hvp->TX_RX(0b0, 0b01111000);
    chip_desc->fuseex = hvp->TX_RX(0b0, 0b01111100) & 0xFF;

    // parsing the sginature bytes

    uint32 signature = (*((uint32_t*)chip_desc->signature)) >> 8;
    for(uint i = 0; i < ArraySize(CHIPS::infos); i++)
    {
        auto info = CHIPS::infos[i];
        if(info.signature == signature)
        {
            chip_desc->info = info;
            signature = 0;
            break;
        }
    }
    if(signature != 0)
    {
        return Response(USB_RESPONSES::CHIPFAULT);
    }

    return Response(USB_RESPONSES::OK, sizeof(ChipDesc), chip_desc.get());
}
Response cmd_write_data(void* data)
{

}
Response cmd_read_data(void* data)
{

}
Response cmd_read_flash(void* data)
{
    returnifnotsetup();
}
Response cmd_write_flash(void* data)
{
    returnifnotsetup();
}
Response cmd_read_eeprom(void* data)
{
    returnifnotsetup();
}
Response cmd_write_eeprom(void* data)
{
    returnifnotsetup();
}
Response cmd_read_fuses(void* data)
{
    returnifnotsetup();
}
Response cmd_write_fuses(void* data)
{
    returnifnotsetup();
}
Response cmd_read_hash_data(void* data)
{
    returnifnotsetup();
}
Response cmd_powered(void* data)
{
    Response res{USB_RESPONSES::OK, 1};
    res.data[0] = !!chippowered;
    return res;
}

void usb_task()
{
    if(!tud_vendor_available())
        return;
    uint8_t data[64];
    tud_vendor_read(&data, 64);
    Command* rcmd = (Command*)data;
    std::vector<std::function<Response(void* data)>> fs = 
    {
        cmd_echo
    };
    Response res;
    if(rcmd->rcmd >= fs.size())
    {
        res.errcode = (uint8_t) USB_RESPONSES::INVALID_COMMAND;
        res.len = 0;
    }
    else if(panicnow)
    {
        res.errcode = (uint8) USB_RESPONSES::FAILURE;
        res.len = 0;
    }
    else if(!is_power_safe)
    {
        res.errcode = (uint8) USB_RESPONSES::NOTREADY;
        res.len = 0;
    }
    else 
        res = fs[rcmd->rcmd](data);
    usb_TX(res);
}