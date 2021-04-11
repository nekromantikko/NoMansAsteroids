#ifndef SDL_TIME_H
#define SDL_TIME_H
#include "../util/typedef.h"

namespace SDL
{
    r64 current_time_in_ms();
    u64 get_ticks();
    u64 ms_to_ticks(r64 t);
    r64 ticks_to_ms(u64 t);
}

#endif // SDL_TIME_H
