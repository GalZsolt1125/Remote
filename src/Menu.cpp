#include "Menu.h"

Menu::Menu(
    TFT_eSPI& display,
    RobotClient& client,
    uint8_t navigationPin,
    uint8_t button1Pin,
    uint8_t button2Pin
)
    : tft(display),
      robotClient(client),
      navigationPin(navigationPin),
      button1Pin(button1Pin),
      button2Pin(button2Pin),
      state(State::CAMERA),
      selectedMainItem(MainItem::BODY),
      selectedBodyItem(BodyItem::BODY_HEIGHT),
      selectedGaitItem(GaitItem::GAIT_TYPE),
      selectedGaitType(GaitTypeItem::TRIPOD),
      bodyHeight(-40.0f),
      stepLength(70.0f),
      stepHeight(0.0f),
      stepPeriod(0.8f),
      lastButton1State(HIGH),
      lastButton2State(HIGH),
      lastInputTime(0),
      lastButtonTime(0)
{
}

void Menu::begin()
{
    state =
        State::CAMERA;

    selectedMainItem =
        MainItem::BODY;

    selectedBodyItem =
        BodyItem::BODY_HEIGHT;

    selectedGaitItem =
        GaitItem::GAIT_TYPE;

    selectedGaitType =
        GaitTypeItem::TRIPOD;

    bodyHeight = -40.0f;
    stepLength = 70.0f;
    stepHeight = 60.0f;
    stepPeriod = 0.8f;
}

bool Menu::isActive() const
{
    return state != State::CAMERA;
}

float Menu::getBodyHeight() const
{
    return bodyHeight;
}

void Menu::setBodyHeight(
    float height
)
{
    bodyHeight =
        constrain(
            height,
            MIN_BODY_HEIGHT,
            MAX_BODY_HEIGHT
        );

    robotClient.setBodyHeight(
        bodyHeight
    );

    if (
        state ==
        State::BODY_HEIGHT
    )
    {
        drawNumericValue(
            bodyHeight,
            "mm",
            0
        );
    }
}

void Menu::adjustBodyHeight(
    float amount
)
{
    setBodyHeight(
        bodyHeight + amount
    );
}

void Menu::adjustStepLength(
    float amount
)
{
    stepLength =
        constrain(
            stepLength + amount,
            MIN_STEP_LENGTH,
            MAX_STEP_LENGTH
        );

    robotClient.setStepLength(
        stepLength
    );

    if (
        state ==
        State::STEP_LENGTH
    )
    {
        drawNumericValue(
            stepLength,
            "mm",
            0
        );
    }
}

void Menu::adjustStepHeight(
    float amount
)
{
    stepHeight =
        constrain(
            stepHeight + amount,
            MIN_STEP_HEIGHT,
            MAX_STEP_HEIGHT
        );

    robotClient.setStepHeight(
        stepHeight
    );

    if (
        state ==
        State::STEP_HEIGHT
    )
    {
        drawNumericValue(
            stepHeight,
            "mm",
            0
        );
    }
}

void Menu::adjustStepPeriod(
    float amount
)
{
    stepPeriod =
        constrain(
            stepPeriod + amount,
            MIN_STEP_PERIOD,
            MAX_STEP_PERIOD
        );

    robotClient.setStepPeriod(
        stepPeriod
    );

    if (
        state ==
        State::STEP_PERIOD
    )
    {
        drawNumericValue(
            stepPeriod,
            "s",
            2
        );
    }
}

bool Menu::buttonPressed(
    uint8_t pin,
    bool& lastState
)
{
    bool currentState =
        digitalRead(pin);

    bool pressed =
        lastState == HIGH &&
        currentState == LOW;

    lastState =
        currentState;

    if (!pressed) {
        return false;
    }

    if (
        millis() - lastButtonTime <
        BUTTON_DEBOUNCE
    )
    {
        return false;
    }

    lastButtonTime =
        millis();

    return true;
}

float Menu::readNavigation()
{
    int value =
        analogRead(
            navigationPin
        );

    float result;

    if (value > 2048)
    {
        result =
            (float)(value - 2048) /
            (4095.0f - 2048.0f);
    }
    else
    {
        result =
            (float)(value - 2048) /
            2048.0f;
    }

    if (fabs(result) < 0.15f) {
        result = 0.0f;
    }

    return constrain(
        result,
        -1.0f,
        1.0f
    );
}

bool Menu::navigationUp()
{
    float value =
        readNavigation();

    if (
        value <= -0.15f &&
        millis() - lastInputTime >=
        INPUT_REPEAT_DELAY
    )
    {
        lastInputTime =
            millis();

        return true;
    }

    return false;
}

bool Menu::navigationDown()
{
    float value =
        readNavigation();

    if (
        value >= 0.15f &&
        millis() - lastInputTime >=
        INPUT_REPEAT_DELAY
    )
    {
        lastInputTime =
            millis();

        return true;
    }

    return false;
}

void Menu::enterMenu()
{
    robotClient.setMotion(
        0.0f,
        0.0f,
        0.0f
    );

    state =
        State::MAIN;

    tft.fillScreen(
        TFT_BLACK
    );

    drawMainMenu();
}

void Menu::leaveMenu()
{
    robotClient.setMotion(
        0.0f,
        0.0f,
        0.0f
    );

    state =
        State::CAMERA;

    tft.fillScreen(
        TFT_BLACK
    );
}

void Menu::drawHeader(
    const char* title
)
{
    tft.fillRect(
        0,
        0,
        320,
        42,
        TFT_DARKGREY
    );

    tft.setTextColor(
        TFT_WHITE,
        TFT_DARKGREY
    );

    tft.setTextSize(1);

    tft.setCursor(
        10,
        12
    );

    tft.print(title);
}

void Menu::drawFooter(
    const char* left,
    const char* right
)
{
    tft.fillRect(
        0,
        214,
        320,
        26,
        TFT_DARKGREY
    );

    tft.setTextColor(
        TFT_WHITE,
        TFT_DARKGREY
    );

    tft.setTextSize(1);

    tft.setCursor(
        8,
        220
    );

    tft.print(left);

    int rightWidth =
        strlen(right) * 6;

    tft.setCursor(
        312 - rightWidth,
        220
    );

    tft.print(right);
}

void Menu::drawSelection(
    int y,
    bool selected
)
{
    if (selected)
    {
        tft.fillRoundRect(
            8,
            y,
            304,
            38,
            5,
            TFT_DARKCYAN
        );
    }
    else
    {
        tft.drawRoundRect(
            8,
            y,
            304,
            38,
            5,
            TFT_DARKGREY
        );
    }
}

void Menu::drawMainMenu()
{
    tft.fillScreen(
        TFT_BLACK
    );

    drawHeader(
        "ROBOT MENU"
    );

    drawSelection(
        60,
        selectedMainItem ==
        MainItem::BODY
    );

    tft.setTextColor(
        TFT_WHITE,
        selectedMainItem ==
        MainItem::BODY
        ? TFT_DARKCYAN
        : TFT_BLACK
    );

    tft.setTextSize(1);

    tft.setCursor(
        22,
        72
    );

    tft.print(
        "BODY"
    );

    drawSelection(
        108,
        selectedMainItem ==
        MainItem::GAIT
    );

    tft.setTextColor(
        TFT_WHITE,
        selectedMainItem ==
        MainItem::GAIT
        ? TFT_DARKCYAN
        : TFT_BLACK
    );

    tft.setCursor(
        22,
        120
    );

    tft.print(
        "GAIT / MOVEMENT"
    );

    drawFooter(
        "B1 BACK",
        "B2 SELECT"
    );
}

void Menu::drawBodyMenu()
{
    tft.fillScreen(
        TFT_BLACK
    );

    drawHeader(
        "BODY"
    );

    drawSelection(
        65,
        selectedBodyItem ==
        BodyItem::BODY_HEIGHT
    );

    tft.setTextColor(
        TFT_WHITE,
        selectedBodyItem ==
        BodyItem::BODY_HEIGHT
        ? TFT_DARKCYAN
        : TFT_BLACK
    );

    tft.setCursor(
        22,
        77
    );

    tft.print(
        "Body Height"
    );

    char text[32];

    snprintf(
        text,
        sizeof(text),
        "%.0f mm",
        bodyHeight
    );

    tft.setCursor(
        250,
        77
    );

    tft.print(text);

    drawFooter(
        "B1 BACK",
        "B2 SELECT"
    );
}

void Menu::drawGaitMenu()
{
    tft.fillScreen(
        TFT_BLACK
    );

    drawHeader(
        "GAIT / MOVEMENT"
    );

    drawSelection(
        48,
        selectedGaitItem ==
        GaitItem::GAIT_TYPE
    );

    tft.setTextColor(
        TFT_WHITE,
        selectedGaitItem ==
        GaitItem::GAIT_TYPE
        ? TFT_DARKCYAN
        : TFT_BLACK
    );

    tft.setCursor(
        22,
        60
    );

    tft.print(
        "Gait Type"
    );

    drawSelection(
        92,
        selectedGaitItem ==
        GaitItem::STEP_LENGTH
    );

    tft.setTextColor(
        TFT_WHITE,
        selectedGaitItem ==
        GaitItem::STEP_LENGTH
        ? TFT_DARKCYAN
        : TFT_BLACK
    );

    tft.setCursor(
        22,
        104
    );

    tft.print(
        "Step Length"
    );

    drawSelection(
        136,
        selectedGaitItem ==
        GaitItem::STEP_HEIGHT
    );

    tft.setTextColor(
        TFT_WHITE,
        selectedGaitItem ==
        GaitItem::STEP_HEIGHT
        ? TFT_DARKCYAN
        : TFT_BLACK
    );

    tft.setCursor(
        22,
        148
    );

    tft.print(
        "Step Height"
    );

    drawSelection(
        180,
        selectedGaitItem ==
        GaitItem::STEP_PERIOD
    );

    tft.setTextColor(
        TFT_WHITE,
        selectedGaitItem ==
        GaitItem::STEP_PERIOD
        ? TFT_DARKCYAN
        : TFT_BLACK
    );

    tft.setCursor(
        22,
        192
    );

    tft.print(
        "Step Period"
    );

    drawFooter(
        "B1 BACK",
        "B2 SELECT"
    );
}

void Menu::drawGaitTypeMenu()
{
    tft.fillScreen(
        TFT_BLACK
    );

    drawHeader(
        "GAIT TYPE"
    );

    drawSelection(
        48,
        selectedGaitType ==
        GaitTypeItem::TRIPOD
    );

    tft.setTextColor(
        TFT_WHITE,
        selectedGaitType ==
        GaitTypeItem::TRIPOD
        ? TFT_DARKCYAN
        : TFT_BLACK
    );

    tft.setCursor(
        22,
        60
    );

    tft.print(
        "TRIPOD"
    );

    drawSelection(
        94,
        selectedGaitType ==
        GaitTypeItem::WAVE
    );

    tft.setTextColor(
        TFT_WHITE,
        selectedGaitType ==
        GaitTypeItem::WAVE
        ? TFT_DARKCYAN
        : TFT_BLACK
    );

    tft.setCursor(
        22,
        106
    );

    tft.print(
        "WAVE"
    );

    drawSelection(
        140,
        selectedGaitType ==
        GaitTypeItem::DANCE
    );

    tft.setTextColor(
        TFT_WHITE,
        selectedGaitType ==
        GaitTypeItem::DANCE
        ? TFT_DARKCYAN
        : TFT_BLACK
    );

    tft.setCursor(
        22,
        152
    );

    tft.print(
        "DANCE"
    );

    drawFooter(
        "B1 BACK",
        "B2 SELECT"
    );
}

void Menu::drawNumericEditor(
    const char* title,
    float value,
    const char* unit,
    uint8_t decimals
)
{
    tft.fillScreen(
        TFT_BLACK
    );

    drawHeader(
        title
    );

    tft.setTextColor(
        TFT_LIGHTGREY,
        TFT_BLACK
    );

    tft.setTextSize(1);

    tft.setCursor(
        120,
        68
    );

    tft.print(
        "VALUE"
    );

    drawNumericValue(
        value,
        unit,
        decimals
    );

    tft.setTextColor(
        TFT_LIGHTGREY,
        TFT_BLACK
    );

    tft.setTextSize(1);

    tft.setCursor(
        122,
        156
    );

    tft.print(
        "UP / DOWN"
    );

    drawFooter(
        "B1 BACK",
        "B2 DONE"
    );
}

void Menu::drawNumericValue(
    float value,
    const char* unit,
    uint8_t decimals
)
{
    tft.fillRect(
        40,
        92,
        240,
        48,
        TFT_BLACK
    );

    char valueText[32];

    snprintf(
        valueText,
        sizeof(valueText),
        decimals == 0
            ? "%.0f %s"
            : "%.2f %s",
        value,
        unit
    );

    tft.setTextColor(
        TFT_WHITE,
        TFT_BLACK
    );

    tft.setTextSize(3);

    int valueWidth =
        strlen(valueText) * 18;

    tft.setCursor(
        160 - valueWidth / 2,
        104
    );

    tft.print(
        valueText
    );

    tft.setTextSize(1);
}

void Menu::drawBodyHeightMenu()
{
    drawNumericEditor(
        "BODY HEIGHT",
        bodyHeight,
        "mm",
        0
    );
}

void Menu::drawStepLengthMenu()
{
    drawNumericEditor(
        "STEP LENGTH",
        stepLength,
        "mm",
        0
    );
}

void Menu::drawStepHeightMenu()
{
    drawNumericEditor(
        "STEP HEIGHT",
        stepHeight,
        "mm",
        0
    );
}

void Menu::drawStepPeriodMenu()
{
    drawNumericEditor(
        "STEP PERIOD",
        stepPeriod,
        "s",
        2
    );
}

void Menu::applyGaitType()
{
    RobotProtocol::GaitTypeWire gait;

    switch (
        selectedGaitType
    )
    {
        case GaitTypeItem::TRIPOD:
            gait =
                RobotProtocol::GaitTypeWire::TRIPOD;
            break;

        case GaitTypeItem::WAVE:
            gait =
                RobotProtocol::GaitTypeWire::WAVE;
            break;

        case GaitTypeItem::DANCE:
            gait =
                RobotProtocol::GaitTypeWire::DANCE;
            break;

        default:
            gait =
                RobotProtocol::GaitTypeWire::TRIPOD;
            break;
    }

    robotClient.setGait(
        gait
    );
}

bool Menu::update()
{
    bool button1 =
        buttonPressed(
            button1Pin,
            lastButton1State
        );

    bool button2 =
        buttonPressed(
            button2Pin,
            lastButton2State
        );

    if (
        state ==
        State::CAMERA
    )
    {
        if (button1)
        {
            enterMenu();
            return true;
        }

        return false;
    }

    if (
        state ==
        State::MAIN
    )
    {
        if (button1)
        {
            leaveMenu();
            return false;
        }

        if (navigationUp())
        {
            selectedMainItem =
                MainItem::BODY;

            drawMainMenu();
        }

        if (navigationDown())
        {
            selectedMainItem =
                MainItem::GAIT;

            drawMainMenu();
        }

        if (button2)
        {
            if (
                selectedMainItem ==
                MainItem::BODY
            )
            {
                state =
                    State::BODY;

                drawBodyMenu();
            }
            else
            {
                state =
                    State::GAIT;

                drawGaitMenu();
            }
        }

        return true;
    }

    if (
        state ==
        State::BODY
    )
    {
        if (button1)
        {
            state =
                State::MAIN;

            drawMainMenu();

            return true;
        }

        if (button2)
        {
            state =
                State::BODY_HEIGHT;

            drawBodyHeightMenu();

            return true;
        }

        return true;
    }

    if (
        state ==
        State::GAIT
    )
    {
        if (button1)
        {
            state =
                State::MAIN;

            drawMainMenu();

            return true;
        }

        if (navigationUp())
        {
            if (
                selectedGaitItem ==
                GaitItem::STEP_LENGTH
            )
            {
                selectedGaitItem =
                    GaitItem::GAIT_TYPE;
            }
            else if (
                selectedGaitItem ==
                GaitItem::STEP_HEIGHT
            )
            {
                selectedGaitItem =
                    GaitItem::STEP_LENGTH;
            }
            else if (
                selectedGaitItem ==
                GaitItem::STEP_PERIOD
            )
            {
                selectedGaitItem =
                    GaitItem::STEP_HEIGHT;
            }

            drawGaitMenu();
        }

        if (navigationDown())
        {
            if (
                selectedGaitItem ==
                GaitItem::GAIT_TYPE
            )
            {
                selectedGaitItem =
                    GaitItem::STEP_LENGTH;
            }
            else if (
                selectedGaitItem ==
                GaitItem::STEP_LENGTH
            )
            {
                selectedGaitItem =
                    GaitItem::STEP_HEIGHT;
            }
            else if (
                selectedGaitItem ==
                GaitItem::STEP_HEIGHT
            )
            {
                selectedGaitItem =
                    GaitItem::STEP_PERIOD;
            }

            drawGaitMenu();
        }

        if (button2)
        {
            switch (
                selectedGaitItem
            )
            {
                case GaitItem::GAIT_TYPE:

                    state =
                        State::GAIT_TYPE;

                    drawGaitTypeMenu();

                    break;

                case GaitItem::STEP_LENGTH:

                    state =
                        State::STEP_LENGTH;

                    drawStepLengthMenu();

                    break;

                case GaitItem::STEP_HEIGHT:

                    state =
                        State::STEP_HEIGHT;

                    drawStepHeightMenu();

                    break;

                case GaitItem::STEP_PERIOD:

                    state =
                        State::STEP_PERIOD;

                    drawStepPeriodMenu();

                    break;
            }
        }

        return true;
    }

    if (
        state ==
        State::GAIT_TYPE
    )
    {
        if (button1)
        {
            state =
                State::GAIT;

            drawGaitMenu();

            return true;
        }

        if (navigationUp())
        {
            if (
                selectedGaitType ==
                GaitTypeItem::WAVE
            )
            {
                selectedGaitType =
                    GaitTypeItem::TRIPOD;
            }
            else if (
                selectedGaitType ==
                GaitTypeItem::DANCE
            )
            {
                selectedGaitType =
                    GaitTypeItem::WAVE;
            }

            drawGaitTypeMenu();
        }

        if (navigationDown())
        {
            if (
                selectedGaitType ==
                GaitTypeItem::TRIPOD
            )
            {
                selectedGaitType =
                    GaitTypeItem::WAVE;
            }
            else if (
                selectedGaitType ==
                GaitTypeItem::WAVE
            )
            {
                selectedGaitType =
                    GaitTypeItem::DANCE;
            }

            drawGaitTypeMenu();
        }

        if (button2)
        {
            applyGaitType();

            state =
                State::GAIT;

            drawGaitMenu();
        }

        return true;
    }

    if (
        state ==
        State::BODY_HEIGHT
    )
    {
        if (button1 || button2)
        {
            state =
                State::BODY;

            drawBodyMenu();

            return true;
        }

        if (navigationUp())
        {
            adjustBodyHeight(
                +1.0f
            );
        }

        if (navigationDown())
        {
            adjustBodyHeight(
                -1.0f
            );
        }

        return true;
    }

    if (
        state ==
        State::STEP_LENGTH
    )
    {
        if (button1 || button2)
        {
            state =
                State::GAIT;

            drawGaitMenu();

            return true;
        }

        if (navigationUp())
        {
            adjustStepLength(
                +1.0f
            );
        }

        if (navigationDown())
        {
            adjustStepLength(
                -1.0f
            );
        }

        return true;
    }

    if (
        state ==
        State::STEP_HEIGHT
    )
    {
        if (button1 || button2)
        {
            state =
                State::GAIT;

            drawGaitMenu();

            return true;
        }

        if (navigationUp())
        {
            adjustStepHeight(
                +1.0f
            );
        }

        if (navigationDown())
        {
            adjustStepHeight(
                -1.0f
            );
        }

        return true;
    }

    if (
        state ==
        State::STEP_PERIOD
    )
    {
        if (button1 || button2)
        {
            state =
                State::GAIT;

            drawGaitMenu();

            return true;
        }

        if (navigationUp())
        {
            adjustStepPeriod(
                +0.05f
            );
        }

        if (navigationDown())
        {
            adjustStepPeriod(
                -0.05f
            );
        }

        return true;
    }

    return true;
}