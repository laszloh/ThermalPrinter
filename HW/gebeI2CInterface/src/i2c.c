#include <C8051F320.h>
#include <stdint.h>

#include "i2c.h"

typedef enum
{
    I2C_READY = 0,
    I2C_MRX,
    I2C_MTX,
    I2C_SRX,
    I2C_STX
} i2c_fsm_t;

static volatile uint8_t i2c_state;
static volatile uint8_t i2c_slarw;
static volatile uint8_t i2c_slaveAddr;
static volatile __bit i2c_sendStop;   // should the transaction end with a stop
static volatile __bit i2c_inRepStart; // in the middle of a repeated start

static void (*i2c_onSlaveTransmit)(void);
static void (*i2c_onSlaveReceive)(uint8_t *, int);

static uint8_t __xdata i2c_masterBuffer[I2C_BUFFER_LENGTH];
static volatile uint8_t i2c_masterBufferIndex;
static volatile uint8_t i2c_masterBufferLength;

static uint8_t __xdata i2c_txBuffer[I2C_BUFFER_LENGTH];
static volatile uint8_t i2c_txBufferIndex;
static volatile uint8_t i2c_txBufferLength;

static uint8_t __xdata i2c_rxBuffer[I2C_BUFFER_LENGTH];
static volatile uint8_t i2c_rxBufferIndex;

static volatile uint8_t i2c_error;

void i2c_init(void)
{
    TMOD |= 0x02;
    CKCON |= 0x04;
    TH0 = 256 - (F_CPU / (3 * I2C_FREQ));

    SMB0CF = 0x80;

    i2c_state = i2c_READY;
    i2c_sendStop = 1;
    i2c_inRepStart = 0;

    EIE1 |= 0x01;
}

void i2c_disable(void)
{
    // disable i2c module
    SMB0CF = 0x00;
}

void i2c_setAddress(uint8_t address)
{
    // set i2c slave address
    i2c_slaveAddr = address << 1;
}

void i2c_setFrequency(uint32_t frequency)
{
    TH0 = 256 - (F_CPU / (3 * frequency));
}

uint8_t i2c_readFrom(uint8_t address, uint8_t *data, uint8_t length, uint8_t sendStop)
{
    uint8_t i;

    // ensure data will fit into buffer
    if (I2C_BUFFER_LENGTH < length)
    {
        return 0;
    }

    // wait until i2c is ready, become master receiver
    uint32_t startMicros = micros();
    while (I2C_READY != i2c_state);

    i2c_state = I2C_MRX;
    i2c_sendStop = sendStop;
    // reset error state (0xFF.. no error occurred)
    i2c_error = 0xFF;

    // initialize buffer iteration vars
    i2c_masterBufferIndex = 0;
    i2c_masterBufferLength = length - 1; // This is not intuitive, read on...
    // On receive, the previously configured ACK/NACK setting is transmitted in
    // response to the received byte before the interrupt is signalled.
    // Therefore we must actually set NACK when the _next_ to last byte is
    // received, causing that NACK to be sent in response to receiving the last
    // expected byte of data.

    // build sla+w, slave device address + w bit
    i2c_slarw = TW_READ;
    i2c_slarw |= address << 1;

    if (true == i2c_inRepStart)
    {
        // if we're in the repeated start state, then we've already sent the start,
        // (@@@ we hope), and the I2C statemachine is just waiting for the address byte.
        // We need to remove ourselves from the repeated start state before we enable interrupts,
        // since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning
        // up. Also, don't enable the START interrupt. There may be one pending from the
        // repeated start that we sent ourselves, and that would really confuse things.
        i2c_inRepStart = false; // remember, we're dealing with an ASYNC ISR
        startMicros = micros();
        do
        {
            TWDR = i2c_slarw;
            if ((i2c_timeout_us > 0ul) && ((micros() - startMicros) > i2c_timeout_us))
            {
                i2c_handleTimeout(i2c_do_reset_on_timeout);
                return 0;
            }
        } while (TWCR & _BV(TWWC));
        TWCR = _BV(I2CNT) | _BV(TWEA) | _BV(TWEN) | _BV(I2CE); // enable INTs, but not START
    }
    else
    {
        // send start condition
        TWCR = _BV(TWEN) | _BV(I2CE) | _BV(TWEA) | _BV(I2CNT) | _BV(TWSTA);
    }

    // wait for read operation to complete
    startMicros = micros();
    while (I2C_MRX == i2c_state)
    {
        if ((i2c_timeout_us > 0ul) && ((micros() - startMicros) > i2c_timeout_us))
        {
            i2c_handleTimeout(i2c_do_reset_on_timeout);
            return 0;
        }
    }

    if (i2c_masterBufferIndex < length)
    {
        length = i2c_masterBufferIndex;
    }

    // copy i2c buffer to data
    for (i = 0; i < length; ++i)
    {
        data[i] = i2c_masterBuffer[i];
    }

    return length;
}

void i2c_interrupt_handler(void) __interrupt 7 __using 2
{
}