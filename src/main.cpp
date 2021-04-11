#include <iostream>
#include <SDL.h>
#include <SDL_vulkan.h>

#define GLM_FORCE_RADIANS
#include <glm/gtx/rotate_vector.hpp>

#include "input/input.h"
#include "rendering/renderer.h"
#include "time/time.h"
#include "rendering/image_loader.h"
#include "asteroids/asteroids.h"

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_HAPTIC);

    ImageLoader::init();
    Renderer::init();
    Input::init();

    //Input::controller_rumble(1, 1.0f);

    bool fullscreen = false;

    //time
    r64 currentTime = Time::current_time_in_ms();

    Asteroids::initialize();

    while (!Input::exit())
    {
        r64 newTime = Time::current_time_in_ms();
        r32 deltaTime = (newTime - currentTime);
        r32 deltaTimeInSeconds = deltaTime / 1000.f;
        currentTime = newTime;

        Renderer::clear_queue();

        Input::refresh();

        Asteroids::play_game(deltaTimeInSeconds);

        Renderer::sort_drawcalls();

        if (!Input::minimized())
            Renderer::draw();
    }

    ImageLoader::deinit();
    Input::deinit();
    Renderer::deinit();


    SDL_Quit();
    return 0;
}
