#ifndef _UTIL_DELAY_BASIC_H_
#define _UTIL_DELAY_BASIC_H_ 1

#include <stdint.h>

/** \ingroup util_delay_basic

    Delay loop using an 8-bit counter \c __count, so up to 256
    iterations are possible.  (The value 256 would have to be passed
    as 0.)  The loop executes three CPU cycles per iteration, not
    including the overhead the compiler needs to setup the counter
    register.

    Thus, at a CPU speed of 1 MHz, delays of up to 768 microseconds
    can be achieved.
*/
inline void _delay_loop_1(uint8_t __count)
{
    (void)__count;
    __asm 
        mov A, DPL
        _d_l_1_loop1:
        djnz ACC, _d_l_1_loop1
    __endasm;
}

/** \ingroup util_delay_basic

    Delay loop using a 16-bit counter \c __count, so up to 65536
    iterations are possible.  (The value 65536 would have to be
    passed as 0.)  The loop executes five CPU cycles per iteration,
    not including the overhead the compiler requires to setup the
    counter register pair.

    Thus, at a CPU speed of 1 MHz, delays of up to about 262.1
    milliseconds can be achieved.
 */
inline void _delay_loop_2(uint16_t __count)
{
    (void)__count;
    __asm
        mov B, DPH                  ; 1
        _d_l_2_loop1:
            mov A, DPL              ; 1
            _d_l_2_loop2:
            djnz ACC, _d_l_2_loop2  ; 2
        djnz B, _d_l_2_loop1        ; 2
    __endasm;
}

#endif /* _UTIL_DELAY_BASIC_H_ */