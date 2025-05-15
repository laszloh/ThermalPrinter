#pragma once

#include <stdint.h>

inline void time_init() {
    // setup Timer 2 as millis timer
    TIMER2CN |= 0x08;
}

inline uint32_t micros() {

}

inline uint32_t millis() {

}