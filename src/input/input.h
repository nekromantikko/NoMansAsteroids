#ifndef INPUT_H
#define INPUT_H
#include "../util/typedef.h"

namespace Input
{
    #define AXIS_COUNT 6
    enum Axis : u8
    {
        AXIS_LEFT_HORIZONTAL,
        AXIS_LEFT_VERTICAL,
        AXIS_RIGHT_HORIZONTAL,
        AXIS_RIGHT_VERTICAL,
        AXIS_TRIGGER_LEFT,
        AXIS_TRIGGER_RIGHT
    };

    #define BUTTON_COUNT 12
    enum Button : u8
    {
        BUTTON_UP,
        BUTTON_RIGHT,
        BUTTON_DOWN,
        BUTTON_LEFT,
        BUTTON_JUMP,
        BUTTON_SHOOT,
        BUTTON_DASH,
        BUTTON_START,
        BUTTON_DEBUG0,
        BUTTON_DEBUG1,
        BUTTON_DEBUG2,
        BUTTON_DEBUG3
    };

    struct DigitalSignal
    {
        bool32 state;
        bool32 prevState;
    };

    #define ANALOG_SENSITIVITY 0.2
    struct AnalogSignal
    {
        r64 state;
        r64 prevState;
    };

    struct State
    {
        AnalogSignal axis[AXIS_COUNT];
        DigitalSignal button[BUTTON_COUNT];
        bool32 minimized;
        bool32 exit;
    };

    void init();
    void deinit();
    void refresh();

    const r64 axis(Axis axis);
    const bool axis_moved(Axis axis);
    const bool axis_released(Axis axis);

    const bool button(Button button);
    const bool button_down(Button button);
    const bool button_up(Button button);

    const bool minimized();
    const bool exit();

    void controller_rumble(r32 strength, r32 length);
}



#endif // INPUT_H
