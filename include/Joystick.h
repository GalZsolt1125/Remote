#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

class Joystick
{
public:
    Joystick(
        uint8_t pin,
        bool invert = false
    );

    void begin();

    float read();

private:
    uint8_t pin;
    bool invert;

    static constexpr int CENTER = 2048;
    static constexpr float DEADZONE = 0.10f;
};

#endif