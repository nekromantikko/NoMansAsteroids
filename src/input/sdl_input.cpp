#include "sdl_input.h"
#include <iostream>

namespace SDL
{
    //TODO: multiple controller support
    SDL_GameController *gameController = NULL;
    SDL_Haptic *haptic = NULL;
}

void SDL::init_controller()
{
    if (SDL_NumJoysticks())
    {
        gameController = SDL_GameControllerOpen(0);
        haptic = SDL_HapticOpenFromJoystick(SDL_GameControllerGetJoystick(gameController));
        SDL_HapticRumbleInit(haptic);
    }
    else gameController = NULL;
}

void SDL::deinit_controller()
{
    if (gameController)
        SDL_GameControllerClose(gameController);
    if (haptic)
    {
        SDL_HapticRumbleStop(haptic);
        SDL_HapticClose(haptic);
    }
}

bool SDL::controller_exists()
{
    return (gameController != 0);
}

void SDL::handle_keyboard_event(SDL_KeyboardEvent &event, Input::State *input)
{
    bool btnState = (event.state == SDL_PRESSED);

    switch(event.keysym.scancode)
    {
    case SDL_SCANCODE_LEFT:
        input->button[Input::BUTTON_LEFT].state = btnState;
        break;
    case SDL_SCANCODE_RIGHT:
        input->button[Input::BUTTON_RIGHT].state = btnState;
        break;
    case SDL_SCANCODE_UP:
        input->button[Input::BUTTON_UP].state = btnState;
        break;
    case SDL_SCANCODE_DOWN:
        input->button[Input::BUTTON_DOWN].state = btnState;
        break;
    case SDL_SCANCODE_SPACE:
        input->button[Input::BUTTON_JUMP].state = btnState;
        break;
    case SDL_SCANCODE_LSHIFT:
        input->button[Input::BUTTON_DASH].state = btnState;
        break;
    case SDL_SCANCODE_ESCAPE:
        input->button[Input::BUTTON_START].state = btnState;
        break;
    case SDL_SCANCODE_D:
        input->button[Input::BUTTON_SHOOT].state = btnState;
        break;
    case SDL_SCANCODE_F1:
        input->button[Input::BUTTON_DEBUG0].state = btnState;
        break;
    case SDL_SCANCODE_F2:
        input->button[Input::BUTTON_DEBUG1].state = btnState;
        break;
    case SDL_SCANCODE_F3:
        input->button[Input::BUTTON_DEBUG2].state = btnState;
        break;
    case SDL_SCANCODE_F4:
        input->button[Input::BUTTON_DEBUG3].state = btnState;
        break;

    default:
        break;
    }
}

void SDL::handle_controller_button_event(SDL_ControllerButtonEvent &event, Input::State *input)
{
    bool btnState = (event.state == SDL_PRESSED);

    switch(event.button)
    {
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        input->button[Input::BUTTON_LEFT].state = btnState;
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        input->button[Input::BUTTON_RIGHT].state = btnState;
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        input->button[Input::BUTTON_UP].state = btnState;
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        input->button[Input::BUTTON_DOWN].state = btnState;
        break;
    case SDL_CONTROLLER_BUTTON_A:
        input->button[Input::BUTTON_JUMP].state = btnState;
        break;
    case SDL_CONTROLLER_BUTTON_X:
        input->button[Input::BUTTON_SHOOT].state = btnState;
        break;
    case SDL_CONTROLLER_BUTTON_START:
        input->button[Input::BUTTON_START].state = btnState;
        break;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        input->button[Input::BUTTON_DEBUG0].state = btnState;
        break;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        input->button[Input::BUTTON_DEBUG1].state = btnState;
        break;
    case SDL_CONTROLLER_BUTTON_Y:
        input->button[Input::BUTTON_DEBUG2].state = btnState;
        break;
    case SDL_CONTROLLER_BUTTON_B:
        input->button[Input::BUTTON_DEBUG3].state = btnState;
        break;
    default:
        break;
    }
}

void SDL::handle_controller_axis_event(SDL_ControllerAxisEvent &event, Input::State *input)
{
    r64 axisState = event.value / 32767.0;
    switch(event.axis)
    {
    case SDL_CONTROLLER_AXIS_LEFTX:
        input->axis[Input::AXIS_LEFT_HORIZONTAL].state = axisState;
        break;
    case SDL_CONTROLLER_AXIS_LEFTY:
        input->axis[Input::AXIS_LEFT_VERTICAL].state = axisState;
        break;
    case SDL_CONTROLLER_AXIS_RIGHTX:
        input->axis[Input::AXIS_RIGHT_HORIZONTAL].state = axisState;
        break;
    case SDL_CONTROLLER_AXIS_RIGHTY:
        input->axis[Input::AXIS_RIGHT_VERTICAL].state = axisState;
        break;
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
        input->axis[Input::AXIS_TRIGGER_LEFT].state = axisState;
        break;
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
        input->axis[Input::AXIS_TRIGGER_RIGHT].state = axisState;
        break;
    default:
        break;
    }
}

void SDL::handle_controller_device_event(SDL_ControllerDeviceEvent &event, Input::State *input)
{
    switch(event.type)
    {
    case SDL_CONTROLLERDEVICEADDED:
        if (event.which == 0)
        {
            if (!gameController)
                init_controller();
        }
        break;
    case SDL_CONTROLLERDEVICEREMOVED:
        if (event.which == 0)
        {
            if (gameController)
                deinit_controller();
        }

        break;
    default:
        break;
    }
}

void SDL::handle_window_event(SDL_WindowEvent &event, Input::State *input)
{
    switch(event.event)
    {
    case SDL_WINDOWEVENT_MINIMIZED:
        input->minimized = true;
        break;
    case SDL_WINDOWEVENT_RESTORED:
        input->minimized = false;
        break;
    default:
        break;
    }
}

void SDL::poll_input(Input::State *input)
{
    bool controllerExists = controller_exists();
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_QUIT:
            input->exit = true;
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            handle_keyboard_event(event.key, input);
            break;
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            if (controllerExists)
                handle_controller_button_event(event.cbutton, input);
            break;
        case SDL_CONTROLLERAXISMOTION:
            if (controllerExists)
                handle_controller_axis_event(event.caxis, input);
            break;
        case SDL_CONTROLLERDEVICEADDED:
        case SDL_CONTROLLERDEVICEREMOVED:
        case SDL_CONTROLLERDEVICEREMAPPED:
            handle_controller_device_event(event.cdevice, input);
        case SDL_WINDOWEVENT:
            handle_window_event(event.window, input);
        default:
            break;
        }
    }
}

//haptic
void SDL::controller_rumble(r32 strength, u32 lengthInMs)
{
    if (haptic)
    {
        SDL_HapticRumblePlay(haptic, strength, lengthInMs);
    }
}
