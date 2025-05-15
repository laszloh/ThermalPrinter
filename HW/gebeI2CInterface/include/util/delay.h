#ifndef _UTIL_DELAY_H_
#define _UTIL_DELAY_H_ 1

#ifndef __DOXYGEN__
#ifndef __HAS_DELAY_CYCLES
#define __HAS_DELAY_CYCLES 1
#endif
#endif /* __DOXYGEN__ */

#include <stdint.h>
#include "delay_basic.h"

/**
   \ingroup util_delay

   Perform a delay of \c __ms milliseconds, using _delay_loop_2().

   The macro F_CPU is supposed to be defined to a
   constant defining the CPU clock frequency (in Hertz).

   The maximal possible delay is 262.14 ms / F_CPU in MHz.

   When the user request delay which exceed the maximum possible one,
   _delay_ms() provides a decreased resolution functionality. In this
   mode _delay_ms() will work with a resolution of 1/10 ms, providing
   delays up to 6.5535 seconds (independent from CPU frequency).  The
   user will not be informed about decreased resolution.

 */
inline void _delay_ms(uint32_t __ms)
{
    const uint32_t __tmp = ((F_CPU) / 4e3) * __ms;
    uint16_t __ticks;
    if (__tmp == 0)
        __ticks = 1;
    else if (__tmp > 65535)
    {
        //	__ticks = requested delay in 1/10 ms
        __ticks = (uint16_t)(__ms * 10);
        while (__ticks)
        {
            // wait 1/10 ms
            _delay_loop_2(((F_CPU) / 4e3) / 10);
            __ticks--;
        }
        return;
    }
    else
        __ticks = (uint16_t)__tmp;
    _delay_loop_2(__ticks);
}

/**
   \ingroup util_delay

   Perform a delay of \c __us microseconds, using _delay_loop_1().

   The macro F_CPU is supposed to be defined to a
   constant defining the CPU clock frequency (in Hertz).

   The maximal possible delay is 768 us / F_CPU in MHz.

   If the user requests a delay greater than the maximal possible one,
   _delay_us() will automatically call _delay_ms() instead.  The user
   will not be informed about this case.
 */
inline void _delay_us(uint32_t __us)
{
    const uint16_t __tmp = ((F_CPU) / 3e6) * __us;
    const uint32_t __tmp2 = ((F_CPU) / 4e6) * __us;
    if (__tmp2 > 65535)
    {
        _delay_ms(__us / 1000.0);
    }
    else if (__tmp > 255)
    {
        uint16_t __ticks = (uint16_t)__tmp2;
        _delay_loop_2(__ticks);
        return;
    }
    uint8_t __ticks = (uint8_t)__tmp;
    if (__tmp == 0)
        __ticks = 1;
    _delay_loop_1(__ticks);
}

#endif /* _UTIL_DELAY_H_ */