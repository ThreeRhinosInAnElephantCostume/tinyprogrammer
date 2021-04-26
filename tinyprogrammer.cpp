#include "hmain.hpp"
enum
{
    // Generic requests
    USBTINY_ECHO,		// 0x00: echo test
    USBTINY_READ,		// 0x01: read byte (wIndex:address)
    USBTINY_WRITE,		// 0x02: write byte (wIndex:address, wValue:value)
    USBTINY_CLR,		// 0x03: clear bit (wIndex:address, wValue:bitno)
    USBTINY_SET,		// 0x04: set bit (wIndex:address, wValue:bitno)
    // Programming requests
    USBTINY_POWERUP,	// 0x05: apply power (wValue:SCK-period, wIndex:RESET)
    USBTINY_POWERDOWN,	// 0x06: remove power from chip
    USBTINY_SPI,		// 0x07: issue SPI command (wValue:c1c0, wIndex:c3c2)
    USBTINY_POLL_BYTES,	// 0x08: set poll bytes for write (wValue:p1p2)
    USBTINY_FLASH_READ,	// 0x09: read flash (wIndex:address)
    USBTINY_FLASH_WRITE,	// 0x0A: write flash (wIndex:address, wValue:timeout)
    USBTINY_EEPROM_READ,	// 0x0B: read eeprom (wIndex:address)
    USBTINY_EEPROM_WRITE,	// 0x0C: write eeprom (wIndex:address, wValue:timeout)
};
struct __attribute__((__packed__)) hostmessage
{
    union
    {
        struct __attribute__((__packed__))
        {
            uint32 total_packet_length;
            struct __attribute__((__packed__)) packet
            {
                uint8 id;
                uint8 cmd;
                uint32 bits;
            } packets[10];
        };
        uint8 buf[64];
    };
    uint len = 0;
    const uint8 minlen = sizeof(uint32_t);
    const uint8 packetlen = sizeof(packet);
};
void HandleHostMessage(hostmessage msg)
{
    for (uint i = 0; i < (msg.len - msg.minlen) / msg.packetlen; i++)
    {
        hostmessage::packet pkt = msg.packets[i];
        uint8_t data[4];
        memcpy(data, &pkt.bits, 4);
        printf("command: %d\n", pkt.cmd);
        uint8 addr = data[0];
        switch (pkt.cmd)
        {
        case USBTINY_ECHO:
        {
            printf("Echoing %d\n", pkt.bits);
            tud_vendor_write(&pkt.bits, sizeof(pkt.bits));
            break;
        }
        case USBTINY_READ:
        {
            tud_vendor_write(usbmemory + addr, 1);
            break;
        }
        case USBTINY_WRITE:
        {
            usbmemory[addr] = data[1];
            break;
        }
        case USBTINY_CLR:
        {
            uint8_t bit = data[1] & 7;
            uint8_t mask = 1 << bit;
            usbmemory[addr] &= ~mask;
            break;
        }
        case USBTINY_SET:
        {
            uint8_t bit = data[1] & 7;
            uint8_t mask = 1 << bit;
            usbmemory[addr] |= mask;
            break;
        }
        case USBTINY_POWERUP:
        {
            gpio_put(PIN::POWER, 1);
            break;
        }
        case USBTINY_POWERDOWN:
        {
            gpio_put(PIN::POWER, 0);
            break;
        }
        case USBTINY_SPI:
        {
            uint32_t out = 0;
            spi_write_read_blocking(TINY_SPI, data, (uint8_t*)&out, 4);
            tud_vendor_write(&out, 4);
            break;
        }
        case USBTINY_POLL_BYTES:
        {
            poll[0] = data[0];
            poll[1] = data[1];
            break;
        }
        case USBTINY_FLASH_READ:
        {

            break;
        }
        case USBTINY_FLASH_WRITE:
        {

            break;
        }
        case USBTINY_EEPROM_READ:
        {

            break;
        }
        case USBTINY_EEPROM_WRITE:
        {

            break;
        }
        }
    }
}
void probe_task(void)
{
    hostmessage msg;
    if (tud_vendor_available())
    {
        uint count = tud_vendor_read(msg.buf, 64);
        if (count == 0)
        {
            return;
        }
        msg.len += count;
    }

    if (msg.len >= msg.minlen)
    {
        if (msg.total_packet_length == msg.len) {
            HandleHostMessage(msg);
        }
    }
}

int main()
{
    stdio_init_all();

    init_out({ PIN::MOSI, PIN::SCK, PIN::POWER }, false);
    init_in({ PIN::MISO });
    gpio_set_function(PIN::MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN::MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN::SCK, GPIO_FUNC_SPI);
    spi_init(TINY_SPI, 100 * 1000);
    tusb_init();

    puts("starting tinyprogrammer");

    while (true)
    {
        probe_task();
    }

    return 0;
}
