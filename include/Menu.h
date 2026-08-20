#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "RobotClient.h"

class Menu
{
public:
    Menu(
        TFT_eSPI& display,
        RobotClient& client,
        uint8_t navigationPin,
        uint8_t button1Pin,
        uint8_t button2Pin
    );

    void begin();

    bool update();

    bool isActive() const;

    float getBodyHeight() const;

    void setBodyHeight(float height);

    void adjustBodyHeight(float amount);

private:
    enum class State
    {
        CAMERA,
        MAIN,
        BODY_HEIGHT,
        GAIT
    };

    enum class MainItem
    {
        BODY_HEIGHT,
        GAIT
    };

    enum class GaitItem
    {
        TRIPOD,
        WAVE,
        DANCE
    };

    TFT_eSPI& tft;
    RobotClient& robotClient;

    uint8_t navigationPin;
    uint8_t button1Pin;
    uint8_t button2Pin;

    State state;
    MainItem selectedMainItem;
    GaitItem selectedGait;

    float bodyHeight;

    bool lastButton1State;
    bool lastButton2State;

    unsigned long lastInputTime;
    unsigned long lastButtonTime;

    static constexpr unsigned long INPUT_REPEAT_DELAY = 140;
    static constexpr unsigned long BUTTON_DEBOUNCE = 180;

    static constexpr float MIN_BODY_HEIGHT = -100.0f;
    static constexpr float MAX_BODY_HEIGHT = 25.0f;

    bool buttonPressed(
        uint8_t pin,
        bool& lastState
    );

    float readNavigation();

    bool navigationUp();

    bool navigationDown();

    void enterMenu();

    void leaveMenu();

    void drawMainMenu();

    void drawBodyHeightMenu();

    void drawBodyHeightValue();

    void drawGaitMenu();

    void drawHeader(
        const char* title
    );

    void drawFooter(
        const char* left,
        const char* right
    );

    void drawSelection(
        int y,
        bool selected
    );
};

#endif