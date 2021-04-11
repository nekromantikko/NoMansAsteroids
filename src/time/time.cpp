#include "time.h"
#include "sdl_time.h"

r64 Time::current_time_in_ms()
{
    return SDL::current_time_in_ms();
}
