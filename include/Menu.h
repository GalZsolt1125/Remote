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

    void setBodyHeight(
        float height
    );

    void adjustBodyHeight(
        float amount
    );

private:

    enum class State
    {
        CAMERA,

        MAIN,

        BODY,

        GAIT,

        GAIT_TYPE,

        BODY_HEIGHT,

        STEP_LENGTH,

        STEP_HEIGHT,

        STEP_PERIOD
    };

    enum class MainItem
    {
        BODY,
        GAIT
    };

    enum class BodyItem
    {
        BODY_HEIGHT
    };

    enum class GaitItem
    {
        GAIT_TYPE,
        STEP_LENGTH,
        STEP_HEIGHT,
        STEP_PERIOD
    };

    enum class GaitTypeItem
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

    BodyItem selectedBodyItem;

    GaitItem selectedGaitItem;

    GaitTypeItem selectedGaitType;

    float bodyHeight;
    float stepLength;
    float stepHeight;
    float stepPeriod;

    bool lastButton1State;
    bool lastButton2State;

    unsigned long lastInputTime;
    unsigned long lastButtonTime;

    static constexpr unsigned long INPUT_REPEAT_DELAY = 140;
    static constexpr unsigned long BUTTON_DEBOUNCE = 180;

    static constexpr float MIN_BODY_HEIGHT = -100.0f;
    static constexpr float MAX_BODY_HEIGHT = 25.0f;

    static constexpr float MIN_STEP_LENGTH = 0.0f;
    static constexpr float MAX_STEP_LENGTH = 200.0f;

    static constexpr float MIN_STEP_HEIGHT = 0.0f;
    static constexpr float MAX_STEP_HEIGHT = 100.0f;

    static constexpr float MIN_STEP_PERIOD = 0.05f;
    static constexpr float MAX_STEP_PERIOD = 5.0f;

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

    void drawBodyMenu();

    void drawGaitMenu();

    void drawGaitTypeMenu();

    void drawBodyHeightMenu();

    void drawStepLengthMenu();

    void drawStepHeightMenu();

    void drawStepPeriodMenu();

    void drawNumericEditor(
        const char* title,
        float value,
        const char* unit,
        uint8_t decimals
    );

    void drawNumericValue(
        float value,
        const char* unit,
        uint8_t decimals
    );

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

    void applyGaitType();

    void adjustStepLength(
        float amount
    );

    void adjustStepHeight(
        float amount
    );

    void adjustStepPeriod(
        float amount
    );
};

#endif