#include "Joystick.h"

Joystick::Joystick(
    uint8_t joystickPin,
    bool joystickInvert
)
    : pin(joystickPin),
      invert(joystickInvert)
{
}

void Joystick::begin()
{
    pinMode(
        pin,
        INPUT
    );
}

float Joystick::read()
{
    int value =
        analogRead(pin);

    float result;

    if (value > CENTER) {

        result =
            (float)(value - CENTER) /
            (4095.0f - CENTER);
    }
    else {

        result =
            (float)(value - CENTER) /
            CENTER;
    }

    if (fabs(result) < DEADZONE) {
        result = 0.0f;
    }

    if (invert) {
        result = -result;
    }

    return constrain(
        result,
        -1.0f,
        1.0f
    );
}