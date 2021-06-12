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

#include "hmain.hpp"
#include "usbcomms.hpp"
#define returnifnotsetup() if(!chippowered || !chip_desc) {return Response((!chippowered)? USB_RESPONSES::NOTPOWERED : USB_RESPONSES::NOTCHECKED);};
uint16 compute_flash_address(uint16 page, uint16 word)
{
    return page * chip_desc->info.flash_page_words + word;
}
uint16 compute_eeprom_address(uint16 page, uint16 bt)
{
    return page * chip_desc->info.eeprom_page_bytes + bt;
}
void power_off()
{
    gpio_put(PIN::SCI, 0);
    gpio_put(PIN::SII, 0);
    gpio_put(PIN::SDI, 0);
    gpio_put(PIN::HIGHVOLT, 0);
    gpio_put(PIN::POWER, 0);
    if(hvp.get())
        hvp.reset();
    chippowered = false;
}
void usb_TX(Response res)
{
    static_assert(sizeof(Response) == 64);
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
    //printf("echoing %s\n", dt->text);
    return Response(USB_RESPONSES::OK, dt->len, (void*) dt->text);
}
Response cmd_prog_ready(void* data)
{
    return Response(USB_RESPONSES::OK, 1, &ispowersafe);
}
Response cmd_chip_powered(void* data)
{
    Response res{USB_RESPONSES::OK, 1};
    res.data[0] = !!chippowered;
    return res;
}
Response cmd_power_on(void* data)
{
    if(chippowered)
        return Response(USB_RESPONSES::OK);
    chip_erased = false;
    hvp.reset();
    if(!ispowersafe)
    {
        return Response{USB_RESPONSES::NOTREADY};
    }
    printf("POWERING ON!\n");
    set_led_mode(LED::PROG_STATUS::GENERICWORK);

    sleep_us(10);
    gpio_put(PIN::POWER, true);
    sleep_us(20);
    gpio_put(PIN::HIGHVOLT, true);
    sleep_us(300);
    hvp = std::make_shared<HVP>(progpio, PIN::SDI, PIN::SII, PIN::SDO, PIN::SCI);
    uint i = 0;
    while(!gpio_get(PIN::SDO))
    {
        if(i > 5000)
        {
            power_off();
            return Response(USB_RESPONSES::CHIPFAULT);
        }
        sleep_us(1);
        i++;
    }
    chippowered = true;
    return Response(USB_RESPONSES::OK);
}
Response cmd_power_off(void* data)
{
    if(!chippowered)
    {
        return Response(USB_RESPONSES::OK);
    }
    set_led_mode(LED::PROG_STATUS::GENERICWORK);
    printf("POWERING OFF!\n");
    power_off();
    chippowered = false;
    return Response(USB_RESPONSES::OK);
}
Response cmd_check(void* data)
{
    if(!chippowered)
    {
        return Response(USB_RESPONSES::NOTPOWERED);
    }
    set_led_mode(LED::PROG_STATUS::GENERICWORK);
    chip_desc = std::make_shared<ChipDesc>();

    // signature
    hvp->TX_RX(0b00001000, 0b01001100);
    for(uint8 i = 0; i < 3; i++)
    {
        hvp->TX_RX(i, 0b00001100);
        //if(i == 0) // fucking hell microchip, fix your documentation!
        hvp->TX_RX(0b0, 0b01101000);
        uint8 v = hvp->TX_RX(0b0, 0b01101100);
        chip_desc->signature[i] = v;
        printf("signature %i: %i\n", (int)i, (int) chip_desc->signature[i]);
    }

    //lock
    hvp->TX_RX(0b00001000, 0b01001100);
    hvp->TX_RX(0b00000000, 0b01111000);
    uint16 bits = hvp->TX_RX(0b00000000, 0b01111000);
    chip_desc->lock1 = !!(bits & (1 << 0));
    chip_desc->lock2 = !!(bits & (1 << 1));
    printf("lock1: %i\nlock2: %i\n", (int) chip_desc->lock1, (int)chip_desc->lock2);

    //calibration
    hvp->TX_RX(0b00001000, 0b01001100);
    hvp->TX_RX(0b0, 0b00001100);
    hvp->TX_RX(0b0, 0b01111000);
    chip_desc->calibration = hvp->TX_RX(0b0, 0b01111100);
    printf("calibration: %i\n", (int) chip_desc->calibration);

    // fuses
    hvp->TX_RX(0b00000100, 0b01001100);
    hvp->TX_RX(0b0, 0b01101000);
    chip_desc->fuselow = hvp->TX_RX(0b0, 0b01111110);
    printf("fuselow: %i\n", (int) chip_desc->fuselow);

    hvp->TX_RX(0b00000100, 0b01001100);
    hvp->TX_RX(0b0, 0b01111010);
    chip_desc->fusehigh = hvp->TX_RX(0b0, 0b01101110);
    printf("fusehigh: %i\n", (int) chip_desc->fusehigh);

    hvp->TX_RX(0b00000100, 0b01001100);
    hvp->TX_RX(0b0, 0b01111000);
    chip_desc->fuseex = hvp->TX_RX(0b0, 0b01111100);
    printf("fusex: %i\n", (int) chip_desc->fuseex);

    // parsing the signature bytes
    printf(" %i %i %i\n", (int)chip_desc->signature[0], (int)chip_desc->signature[1], (int)chip_desc->signature[2]);
    volatile uint32 signature = (chip_desc->signature[0] << 16) | (chip_desc->signature[1]  << 8) | (chip_desc->signature[2]);
    for(uint i = 0; i < ArraySize(CHIPS::infos); i++)
    {
        auto info = CHIPS::infos[i];
        if(info.signature == signature)
        {
            chip_desc->info = info;
            goto foundchip;
        }
    }
    chip_desc.reset();
    printf("invalid/unsupported chip signature %d!\n", (uint)signature);
    return Response(USB_RESPONSES::CHIPFAULT);
    foundchip:;
    printf("Detected chip: %i\n", (int)chip_desc->info.id);
    return Response(USB_RESPONSES::OK, sizeof(ChipDesc), chip_desc.get());
}
Response cmd_chip_erase(void* data)
{
    returnifnotsetup();
    set_led_mode(LED::PROG_STATUS::WRITING);
    chip_erased = false;
    hvp->TX_RX(0b10000000, 0b01001100);
    hvp->TX_RX(0b0, 0b01100100);
    hvp->TX_RX(0b0, 0b01101100);
    int i = 0;
    if(!hvp->wait_till_ready(100000))
        return Response(USB_RESPONSES::CHIPFAULT);
    hvp->TX_NOOP();
    chip_erased = true;
    printf("chip erased\n");
    return Response(USB_RESPONSES::OK);
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
    set_led_mode(LED::PROG_STATUS::READING);
    printf("read %i bytes from usbmemory\n", (int)cmd->len);
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
    set_led_mode(LED::PROG_STATUS::WRITING);
    memcpy(usbmemory + cmd->address, cmd->data, cmd->len);
    printf("written %i bytes from usbmemory\n", (int)cmd->len);
    return Response(USB_RESPONSES::OK, cmd->len, usbmemory + cmd->address); // return for potential validation
}
Response cmd_read_hash_data(void* data)
{
    set_led_mode(LED::PROG_STATUS::READING);
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
    printf("hashed %i bytes of data: %ulli\n", (int)cmd->len, ret.hash);
    return Response(USB_RESPONSES::OK, sizeof(ret), &ret);
}
Response cmd_read_flash(void* data)
{
    returnifnotsetup();
    auto cmd = (RWPaged*)data;
    if(cmd->memaddr + cmd->npages*chip_desc->info.flash_page_bytes < cmd->memaddr)
    {
        return Response(USB_RESPONSES::INVALID_RANGE);
    }
    if(cmd->npages*chip_desc->info.flash_page_bytes > chip_desc->info.flash_bytes)
    {
        return Response(USB_RESPONSES::INVALID_ARGUMENT);
    }
    set_led_mode(LED::PROG_STATUS::READING);
    uint16 dest = cmd->memaddr;
    bool b = true;
    int si = 0;
    hvp->TX_NOOP();
    hvp->TX_RX(0b00000010, 0b01001100);
    for(uint16 i = 0; i < cmd->npages; i++ )
    {
        int page = cmd->startpage + i;
        for(uint16 ii = 0; ii < chip_desc->info.flash_page_words; ii++)
        {
            uint16 addr = compute_flash_address(page, ii);
            hvp->TX_RX(addr & 0xFF, 0b00001100);
            if(ii == 0)
                hvp->TX_RX((addr & 0xFF00) >> 8, 0b00011100);
            hvp->TX_RX(0b0, 0b01101000);
            uint8 l = hvp->TX_RX(0b0, 0b01101100);
            hvp->TX_RX(0b0, 0b01111000);
            uint8 h = hvp->TX_RX(0b0, 0b01111100);
            usbmemory[dest] = l;
            usbmemory[dest+1] = h;
            dest += 2;
        }
    }
    hvp->TX_NOOP();
    sleep_us(1);
    printf("read %i pages from flash\n", (int)cmd->npages);
    return Response(USB_RESPONSES::OK);
}
Response cmd_write_flash(void* data)
{
    returnifnotsetup();
    if(!chip_erased)
        return Response(USB_RESPONSES::NOTERASED);
    auto cmd = (RWPaged*)data;
    if(cmd->memaddr + cmd->npages*chip_desc->info.flash_page_bytes < cmd->memaddr)
    {
        return Response(USB_RESPONSES::INVALID_RANGE);
    }
    if(cmd->npages*chip_desc->info.flash_page_bytes > chip_desc->info.flash_bytes)
    {
        return Response(USB_RESPONSES::INVALID_ARGUMENT);
    }
    set_led_mode(LED::PROG_STATUS::WRITING);
    uint16 source = cmd->memaddr;
    hvp->TX_NOOP();
    hvp->TX_RX(0b0010000, 0b01001100);
    for(int i = 0; i < cmd->npages; i++)
    {
        uint16 addr = 0;
        for(int ii = 0; ii < chip_desc->info.flash_page_words;ii++)
        {
            addr = compute_flash_address(i+cmd->startpage, ii);
            hvp->TX_RX(addr & 0xFF, 0b00001100);
            hvp->TX_RX(usbmemory[source], 0b00101100);
            hvp->TX_RX(usbmemory[source+1], 0b00111100);
            hvp->TX_RX(0b0, 0b01111101);
            hvp->TX_RX(0b0, 0b01111100);
            source += 2;
        }
        hvp->TX_RX((addr & 0xFF00) >> 8, 0b00011100);
        hvp->TX_RX(0b0, 0b01100100);
        hvp->TX_RX(0b0, 0b01101100);
        if(!hvp->wait_till_ready(10000))
            return Response(USB_RESPONSES::CHIPFAULT);
    }
    hvp->TX_NOOP();
    sleep_us(1);
    printf("written %i pages from flash\n", (int)cmd->npages);
    return Response(USB_RESPONSES::OK);
}
Response cmd_read_eeprom(void* data)
{
    returnifnotsetup();
    auto cmd = (RWPaged*) data;

    if(cmd->memaddr + cmd->npages*chip_desc->info.eeprom_page_bytes < cmd->memaddr)
    {
        return Response(USB_RESPONSES::INVALID_RANGE);
    }
    if(cmd->npages*chip_desc->info.eeprom_page_bytes > chip_desc->info.eeprom_bytes)
    {
        return Response(USB_RESPONSES::INVALID_ARGUMENT);
    }
    set_led_mode(LED::PROG_STATUS::READING);

    uint16 dest = cmd->memaddr;
    hvp->TX_NOOP();
    hvp->TX_RX(0b0000011, 0b01001100);

    for(uint i = 0; i < cmd->npages; i++)
    {
        for(uint ii = 0; ii < chip_desc->info.eeprom_page_bytes; ii++)
        {
            uint16 addr = compute_eeprom_address(i, ii);
            hvp->TX_RX(addr & 0xFF, 0b00001100);
            if(ii == 0)
                hvp->TX_RX(addr >> 8, 0b00011100);
            hvp->TX_RX(0b0, 0b01101000);
            uint8 v = hvp->TX_RX(0b0, 0b01101100);
            usbmemory[dest] = v;
            dest++;
        }
    }
    hvp->TX_NOOP();
    printf("read %i pages from eeprom\n", (int)cmd->npages);
    return Response(USB_RESPONSES::OK);
}
Response cmd_write_eeprom(void* data)
{
    returnifnotsetup();
    if(!chip_erased)
        return Response(USB_RESPONSES::NOTERASED);

    auto cmd = (RWPaged*) data;

    if(cmd->memaddr + cmd->npages*chip_desc->info.flash_page_bytes < cmd->memaddr)
    {
        return Response(USB_RESPONSES::INVALID_RANGE);
    }
    if(cmd->npages*chip_desc->info.flash_page_bytes > chip_desc->info.flash_bytes)
    {
        return Response(USB_RESPONSES::INVALID_ARGUMENT);
    }
    set_led_mode(LED::PROG_STATUS::WRITING);

    uint16 source = cmd->memaddr;
    hvp->TX_NOOP();
    hvp->TX_RX(0b00010001, 0b01001100);

    for(uint i = 0; i < cmd->npages; i++)
    {
        for(uint ii = 0; ii < chip_desc->info.eeprom_page_bytes; ii++)
        {
            uint16 addr = compute_eeprom_address(i, ii);
            hvp->TX_RX(addr & 0xFF, 0b00001100);
            hvp->TX_RX(addr >> 8, 0b00011100);
            hvp->TX_RX(usbmemory[source], 0b01101101);
            hvp->TX_RX(0b0, 0b01100100);
            hvp->TX_RX(0b0, 0b01101100);
            if(!hvp->wait_till_ready(5000))
                return Response(USB_RESPONSES::CHIPFAULT);
            source++;
        }
    }
    hvp->TX_NOOP();
    printf("written %i pages from eeprom\n", (int)cmd->npages);
    return Response(USB_RESPONSES::OK);
}
Response cmd_read_fuses(void* data)
{
    returnifnotsetup();
    set_led_mode(LED::PROG_STATUS::READING);
    struct
    {
        uint8 low;
        uint8 high;
        uint8 extended;
    }ret = {0};
    // fuses
    hvp->TX_RX(0b00000100, 0b01001100);
    hvp->TX_RX(0b0, 0b01101000);
    ret.low = hvp->TX_RX(0b0, 0b01111110);

    hvp->TX_RX(0b00000100, 0b01001100);
    hvp->TX_RX(0b0, 0b01111010);
    ret.high = hvp->TX_RX(0b0, 0b01101110);

    hvp->TX_RX(0b00000100, 0b01001100);
    hvp->TX_RX(0b0, 0b01111000);
    ret.extended = hvp->TX_RX(0b0, 0b01111100);

    printf("read fuses l%i h%i e%i\n", (int)ret.low, (int)ret.high, (int)ret.extended);

    return Response(USB_RESPONSES::OK, sizeof(ret), &ret);
}
Response cmd_write_fuses(void* data)
{
    returnifnotsetup();
    set_led_mode(LED::PROG_STATUS::WRITING);
    auto cmd = (WriteFuses*)data;
    hvp->TX_RX(0b01000000, 0b01001100);
    hvp->TX_RX(cmd->low, 0b00101100);
    hvp->TX_RX(0b0, 0b01100100);
    hvp->TX_RX(0b0, 0b01101100);
    if(!hvp->wait_till_ready(5000))
        return Response(USB_RESPONSES::CHIPFAULT);

    hvp->TX_RX(0b01000000, 0b01001100);
    hvp->TX_RX(cmd->high, 0b00101100);
    hvp->TX_RX(0b0, 0b01110100);
    hvp->TX_RX(0b0, 0b01111100);
    if(!hvp->wait_till_ready(5000))
        return Response(USB_RESPONSES::CHIPFAULT);

    hvp->TX_RX(0b01000000, 0b01001100);
    hvp->TX_RX(cmd->extended, 0b00101100);
    hvp->TX_RX(0b0, 0b01100110);
    hvp->TX_RX(0b0, 0b01101110);
    if(!hvp->wait_till_ready(5000))
        return Response(USB_RESPONSES::CHIPFAULT);
    
    printf("set fuses to l%i h%i e%i\n", (int)cmd->low, (int)cmd->high, (int)cmd->extended);
    return Response(USB_RESPONSES::OK);
}
Response cmd_write_lock(void* data)
{
    returnifnotsetup();
    set_led_mode(LED::PROG_STATUS::WRITING);
    if(!chip_erased)
        return Response(USB_RESPONSES::NOTERASED);
    auto cmd = (WriteLock*)data;
    hvp->TX_RX(0b00100000, 0b01001100);
    hvp->TX_RX(cmd->lock, 0b00101100);
    hvp->TX_RX(0b0, 0b01100100);
    hvp->TX_RX(0b0, 0b01101100);
    if(!hvp->wait_till_ready(5000))
        return Response(USB_RESPONSES::CHIPFAULT);
    
    printf("written lock byte %i\n", (int)cmd->lock);
    return Response(USB_RESPONSES::OK);
}
Response cmd_read_calibration(void* data)
{
    returnifnotsetup();
    set_led_mode(LED::PROG_STATUS::READING);
    hvp->TX_RX(0b00001000, 0b01001100);
    hvp->TX_RX(0b0, 0b00001100);
    hvp->TX_RX(0b0, 0b01111000);
    uint8 ret = hvp->TX_RX(0b0, 0b01111100);

    printf("read calibration byte %i\n", (int)ret);
    return Response(USB_RESPONSES::OK, 1, &ret);
}
Response cmd_was_erased(void* data)
{
    return Response(USB_RESPONSES::OK, 1, &chip_erased);
}
bool usb_task()
{
    if(!tud_vendor_available())
        return false;
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
        cmd_write_fuses,

        cmd_write_lock,

        cmd_read_calibration,

        cmd_was_erased
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
    else if(!ispowersafe)
    {
        res.errcode = (uint8) USB_RESPONSES::NOTREADY;
        res.len = 0;
    }
    else 
    {
        res = fs[rcmd->rcmd](data);
    }
    if(DEBUG || res.errcode != (int)USB_RESPONSES::OK)
    {
        printf("received command %i, responded with %i with len %i\n", (int)rcmd->rcmd, (int)res.errcode, (int)res.len);
    }
    usb_TX(res);
    return true;
}