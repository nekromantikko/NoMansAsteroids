#include "input.h"
#include "sdl_input.h"
#include <cmath>

namespace Input
{
    u32 controllerCount;

    State inputState;

}

void Input::init()
{
    SDL::init_controller();
}

void Input::deinit()
{
    SDL::deinit_controller();
}

void Input::refresh()
{
    for (int i = 0; i < AXIS_COUNT; i++)
    {
        inputState.axis[i].prevState = inputState.axis[i].state;
    }

    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        inputState.button[i].prevState = inputState.button[i].state;
    }

    SDL::poll_input(&inputState);
}

const r64 Input::axis(Axis axis)
{
    r64 result = inputState.axis[axis].state;
    if (std::abs(result) < ANALOG_SENSITIVITY)
        result = 0.0f;

    return result;
}
const bool Input::axis_moved(Axis axis)
{
    r64 state = inputState.axis[axis].state;
    r64 prevState = inputState.axis[axis].prevState;
    return std::abs(state) > ANALOG_SENSITIVITY && std::abs(prevState) <= ANALOG_SENSITIVITY;
}
const bool Input::axis_released(Axis axis)
{
    r64 state = inputState.axis[axis].state;
    r64 prevState = inputState.axis[axis].prevState;
    return std::abs(state) <= ANALOG_SENSITIVITY && std::abs(prevState) > ANALOG_SENSITIVITY;
}

const bool Input::button(Button button)
{
    return inputState.button[button].state > 0;
}
const bool Input::button_down(Button button)
{
    bool state = inputState.button[button].state;
    bool prevState = inputState.button[button].prevState;
    return state && !prevState;
}
const bool Input::button_up(Button button)
{
    bool state = inputState.button[button].state;
    bool prevState = inputState.button[button].prevState;
    return !state && prevState;
}

const bool Input::minimized()
{
    return inputState.minimized;
}

const bool Input::exit()
{
    return inputState.exit;
}

void Input::controller_rumble(r32 strength, r32 length)
{
    SDL::controller_rumble(strength, length * 1000);
}

