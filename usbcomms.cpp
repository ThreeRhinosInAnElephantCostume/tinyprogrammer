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
    printf("echoing %s\n", dt->text);
    return Response(USB_RESPONSES::OK, dt->len, (void*) dt->text);
}
Response cmd_prog_ready(void* data)
{
    return Response(USB_RESPONSES::OK, 1, &is_power_safe);
}
Response cmd_chip_powered(void* data)
{
    Response res{USB_RESPONSES::OK, 1};
    res.data[0] = !!chippowered;
    return res;
}
Response cmd_power_on(void* data)
{
    chip_erased = false;
    hvp = nullptr;
    if(!is_power_safe)
    {
        return Response{USB_RESPONSES::NOTREADY};
    }
    printf("POWERING ON!\n");
    init_out({PIN::SDI, PIN::SII, PIN::SDO, PIN::SCI}, false);
    init_out({PIN::POWER, PIN::HIGHVOLT}, false);
    sleep_us(10);
    gpio_put(PIN::POWER, true);
    sleep_us(20);
    gpio_put(PIN::HIGHVOLT, true);
    sleep_us(10);
    init_in(PIN::SDO);
    sleep_us(300);
    hvp = std::make_shared<HVP>(progpio, PIN::SDI, PIN::SII, PIN::SDO, PIN::SCI);
    chippowered = true;
    return Response(USB_RESPONSES::OK);
}
Response cmd_power_off(void* data)
{
    if(!chippowered)
    {
        return Response(USB_RESPONSES::OK);
    }
    hvp = nullptr;
    printf("POWERING OFF!\n");
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
    chip_desc = std::shared_ptr<ChipDesc>();

    // lock
    hvp->TX_RX(0b00001000, 0b01001100);
    hvp->TX_RX(0b00000000, 0b01111000);
    uint16 bits = hvp->TX_RX(0b00000000, 0b01111000);
    chip_desc->lock1 = !!(bits & (1 << 7));
    chip_desc->lock2 = !!(bits & (1 << 6));
    printf("lock1: %i\nlock2: %i\n", (int) chip_desc->lock1, (int)chip_desc->lock2);
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
        printf("signature %i: %i\n", (int)i, (int) chip_desc->signature[i]);
    }

    // calibration
    hvp->TX_RX(0b00001000, 0b01001100);
    hvp->TX_RX(0b0, 0b00001100);
    hvp->TX_RX(0b0, 0b01111000);
    chip_desc->calibration = hvp->TX_RX(0b0, 0b01111100) & 0xFF;
    printf("calibration: %i\n", (int) chip_desc->calibration);

    // fuses
    hvp->TX_RX(0b00000100, 0b01001100);
    hvp->TX_RX(0b0, 0b01101000);
    chip_desc->fuselow = hvp->TX_RX(0b0, 0b01111110) & 0xFF;
    printf("fuselow: %i\n", (int) chip_desc->fuselow);

    hvp->TX_RX(0b00000100, 0b01001100);
    hvp->TX_RX(0b0, 0b01111010);
    chip_desc->fusehigh = hvp->TX_RX(0b0, 0b01101110) & 0xFF;
    printf("fusehigh: %i\n", (int) chip_desc->fusehigh);

    hvp->TX_RX(0b00000100, 0b01001100);
    hvp->TX_RX(0b0, 0b01111000);
    chip_desc->fuseex = hvp->TX_RX(0b0, 0b01111100) & 0xFF;
    printf("fusex: %i\n", (int) chip_desc->fuseex);

    // parsing the signature bytes

    volatile uint32 signature = (*((uint32_t*)chip_desc->signature)) >> 8;
    for(uint i = 0; i < ArraySize(CHIPS::infos); i++)
    {
        auto info = CHIPS::infos[i];
        if(info.signature == signature)
        {
            chip_desc->info = info;
            goto foundchip;
        }
    }
    chip_desc = nullptr;
    printf("invalid/unsupported chip signature %d!\n", (uint)signature);
    return Response(USB_RESPONSES::CHIPFAULT);
    foundchip:;
    printf("Detected chip: %i\n", (int)chip_desc->info.id);
    return Response(USB_RESPONSES::OK, sizeof(ChipDesc), chip_desc.get());
}
Response cmd_chip_erase(void* data)
{
    returnifnotsetup();
    chip_erased = true;
}
Response cmd_read_data(void* data)
{
    auto cmd = (ReadData*)data;
    if(cmd->len > 62)
    {
        return Response(USB_RESPONSES::INVALID_ARGUMENT);
    }
    if(cmd->len + cmd->address < cmd->address)
    {
        return Response(USB_RESPONSES::INVALID_RANGE);
    }
    return Response(USB_RESPONSES::OK, cmd->len, usbmemory + cmd->address);
}
Response cmd_write_data(void* data)
{
    auto cmd = (WriteData*)data;
    if(cmd->len > 60)
    {
        return Response(USB_RESPONSES::INVALID_ARGUMENT);
    }
    if(cmd->len + cmd->address < cmd->address)
    {
        return Response(USB_RESPONSES::INVALID_RANGE);
    }
    memcpy(usbmemory + cmd->address, cmd->data, cmd->len);
    return Response(USB_RESPONSES::OK, cmd->len, usbmemory + cmd->address); // return for potential validation
}
Response cmd_read_hash_data(void* data)
{
    auto cmd = (HashData*)data;
    if(cmd->address + cmd->len < cmd->address)
    {
        return Response(USB_RESPONSES::INVALID_RANGE);
    }
    struct __attribute__((__packed__))
    {
        uint64 hash=0;
    }ret;
    for(uint16 i = 0; i < cmd->len; i++)
    {
        ((uint8*)&ret.hash)[i % 8] ^= usbmemory[cmd->address + i];
    }
    return Response(USB_RESPONSES::OK, sizeof(ret), &ret);
    
}
Response cmd_read_flash(void* data)
{
    returnifnotsetup();
    auto cmd = (ReadFlash*)data;
    if(cmd->destination + cmd->npages*chip_desc->info.flash_page_bytes < cmd->destination)
    {
        return Response(USB_RESPONSES::INVALID_RANGE);
    }
    if(cmd->npages*chip_desc->info.flash_page_bytes > chip_desc->info.flash_bytes)
    {
        return Response(USB_RESPONSES::INVALID_ARGUMENT);
    }
    hvp->TX_RX(0b00000010, 0b01001100);
    uint16 dest = cmd->destination;
    for(uint16 i = 0; i < cmd->npages; i++ )
    {
        for(uint16 ii = 0; ii < chip_desc->info.flash_page_words; ii++)
        {
            hvp->TX_RX((cmd->startpage+i) & 0xFF, 0b00001100);
            if(ii == 0)
                hvp->TX_RX((!!((cmd->startpage+i) & 0x100)), 0b00011100);
            hvp->TX_RX(0b0, 0b01101000);
            uint8 l = hvp->TX_RX(0b0, 0b01101100);
            hvp->TX_RX(0b0, 0b01111000);
            uint8 h = hvp->TX_RX(0b0, 0b01111100);
            usbmemory[dest] = h;
            usbmemory[dest+1] = l;
            dest += 2;
            printf("%x  %x\n", h, l);
        }
    }
    return Response(USB_RESPONSES::OK);
}
Response cmd_write_flash(void* data)
{
    returnifnotsetup();
    if(!chip_erased)
        return Response(USB_RESPONSES::NOTERASED);
}
Response cmd_read_eeprom(void* data)
{
    returnifnotsetup();
}
Response cmd_write_eeprom(void* data)
{
    returnifnotsetup();
    if(!chip_erased)
        return Response(USB_RESPONSES::NOTERASED);
}
Response cmd_read_fuses(void* data)
{
    returnifnotsetup();
}
Response cmd_write_fuses(void* data)
{
    returnifnotsetup();
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
        cmd_echo,

        cmd_prog_ready,
        cmd_chip_powered,

        cmd_power_on,
        cmd_power_off,
        
        cmd_check,

        cmd_chip_erase,
        
        cmd_read_data,
        cmd_write_data,
        cmd_read_hash_data,

        cmd_read_flash,
        cmd_write_flash,

        cmd_read_eeprom,
        cmd_write_eeprom,

        cmd_read_fuses,
        cmd_write_fuses
    };
    Response res;
    printf("cmd: %i\n", (int)rcmd->rcmd);
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
    {
        res = fs[rcmd->rcmd](data);
    }
    printf("res: %i\n", (int)res.errcode);
    usb_TX(res);
}