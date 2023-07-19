#ifndef _I2C_H_
#define _I2C_H_

#include <stdbool.h>

#ifndef I2C_FREQ
#define I2C_FREQ 100000UL
#endif

#ifndef I2C_BUFFER_LENGTH
#define I2C_BUFFER_LENGTH (0x40)
#endif

void i2c_init(void);
void i2c_disable(void);
void i2c_setAddress(uint8_t);
void i2c_setFrequency(uint32_t);
uint8_t i2c_readFrom(uint8_t, uint8_t *, uint8_t, uint8_t);
uint8_t i2c_writeTo(uint8_t, uint8_t *, uint8_t, uint8_t, uint8_t);
uint8_t i2c_transmit(const uint8_t *, uint8_t);
void i2c_attachSlaveRxEvent(void (*)(uint8_t *, int));
void i2c_attachSlaveTxEvent(void (*)(void));
void i2c_reply(uint8_t);
void i2c_stop(void);
void i2c_releaseBus(void);
void i2c_setTimeoutInMicros(uint32_t, bool);
void i2c_handleTimeout(bool);
bool i2c_manageTimeoutFlag(bool);

#endif // _I2C_H_