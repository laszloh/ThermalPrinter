#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <stdbool.h>

void serial_init();
void serial_putc(unsigned char);
unsigned char serial_getc();
bool serial_avaliable();
bool serial_tx_empty();

#endif // _SERIAL_H_