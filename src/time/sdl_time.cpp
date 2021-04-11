#include "sdl_time.h"
#include <SDL.h>
#include <cmath>

r64 SDL::current_time_in_ms()
{
    return SDL_GetPerformanceCounter() / (SDL_GetPerformanceFrequency() / 1000.0);
}
u64 SDL::get_ticks()
{
    return SDL_GetPerformanceCounter();
}
u64 SDL::ms_to_ticks(r64 t)
{
    r64 timeInSeconds = t / 1000.f;
    return std::round(timeInSeconds * SDL_GetPerformanceFrequency());
}
r64 SDL::ticks_to_ms(u64 t)
{
    r64 ticksPerSecond = t * 1000;
    return ticksPerSecond / SDL_GetPerformanceFrequency();
}
