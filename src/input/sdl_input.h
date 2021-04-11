#ifndef INPUT_SDL_H
#define INPUT_SDL_H
#include "input.h"
#include <SDL.h>

namespace SDL
{
    void init_controller();
    void deinit_controller();
    bool controller_exists();

    void handle_keyboard_event(SDL_KeyboardEvent &event, Input::State *input);
    void handle_controller_button_event(SDL_ControllerButtonEvent &event, Input::State *input);
    void handle_controller_axis_event(SDL_ControllerAxisEvent &event, Input::State *input);
    void handle_controller_device_event(SDL_ControllerDeviceEvent &event, Input::State *input);
    void handle_window_event(SDL_WindowEvent &event, Input::State *input);

    void poll_input(Input::State *input);

    //haptic
    void controller_rumble(r32 strength, u32 lengthInMs);
}

#endif // INPUT_SDL_H
