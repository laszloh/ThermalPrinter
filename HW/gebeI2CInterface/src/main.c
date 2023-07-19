#include <stdint.h>
#include <C8051F320.h>

#include "serial.h"
#include "util/delay.h"

void gpio_init()
{
    // P0.0  -  Skipped,     Push-Pull,  Digital
    // P0.1  -  Skipped,     Open-Drain, Digital
    // P0.2  -  Skipped,     Push-Pull,  Digital
    // P0.3  -  Skipped,     Open-Drain, Digital
    // P0.4  -  TX0 (UART0), Open-Drain, Digital
    // P0.5  -  RX0 (UART0), Open-Drain, Digital
    // P0.6  -  Skipped,     Open-Drain, Digital
    // P0.7  -  Skipped,     Open-Drain, Digital

    // P1.0  -  Skipped,     Push-Pull,  Digital
    // P1.1  -  SDA (SMBus), Push-Pull,  Digital
    // P1.2  -  SCL (SMBus), Open-Drain, Digital
    // P1.3  -  Unassigned,  Open-Drain, Digital
    // P1.4  -  Unassigned,  Push-Pull,  Digital
    // P1.5  -  Unassigned,  Push-Pull,  Digital
    // P1.6  -  Unassigned,  Push-Pull,  Digital
    // P1.7  -  Unassigned,  Open-Drain, Digital
    // P2.0  -  Unassigned,  Open-Drain, Digital
    // P2.1  -  Unassigned,  Open-Drain, Digital
    // P2.2  -  Unassigned,  Push-Pull,  Digital
    // P2.3  -  Unassigned,  Open-Drain, Digital

    P0MDOUT = 0x05;
    P1MDOUT = 0x73;
    P2MDOUT = 0x04;
    P0SKIP = 0xCF;
    P1SKIP = 0x01;

    XBR0 = 0x05;
    XBR1 = 0x40;
}

void clock_init()
{
    // setup 12MHz internal clock
    OSCICN = 0x83;

    // enable clock multiplier for usb
    CLKMUL = 0x80;
    _delay_us(10 * 2); // we are running here only with 12 MHz
    CLKMUL |= 0xC0;
    while ((CLKMUL & 0x20) == 0)
        ;
    // switch uC to 24 MHz
    CLKSEL = 0x02;
    OSCICN = 0x83;
}

void setup()
{
    clock_init();
    gpio_init();

    serial_init();
}

void main()
{
    setup();

    while (1)
    {
        P1 = 0xFF;
        _delay_ms(500);
        P1 = 0x00;
        _delay_ms(500);
    }
}
