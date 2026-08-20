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
      selectedMainItem(MainItem::BODY_HEIGHT),
      selectedGait(GaitItem::TRIPOD),
      bodyHeight(-20.0f),
      lastButton1State(HIGH),
      lastButton2State(HIGH),
      lastInputTime(0),
      lastButtonTime(0)
{
}

void Menu::begin()
{
    state = State::CAMERA;
    selectedMainItem = MainItem::BODY_HEIGHT;
    selectedGait = GaitItem::TRIPOD;
    bodyHeight = -20.0f;
}

bool Menu::isActive() const
{
    return state != State::CAMERA;
}

float Menu::getBodyHeight() const
{
    return bodyHeight;
}

void Menu::setBodyHeight(float height)
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

    if (state == State::BODY_HEIGHT) {
        drawBodyHeightValue();
    }
}

void Menu::adjustBodyHeight(float amount)
{
    setBodyHeight(
        bodyHeight + amount
    );
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

    lastState = currentState;

    if (!pressed) {
        return false;
    }

    if (
        millis() - lastButtonTime <
        BUTTON_DEBOUNCE
    ) {
        return false;
    }

    lastButtonTime =
        millis();

    return true;
}

float Menu::readNavigation()
{
    int value =
        analogRead(navigationPin);

    float result;

    if (value > 2048) {
        result =
            (float)(value - 2048) /
            (4095.0f - 2048.0f);
    }
    else {
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

    if (value <= -0.15f) {
        if (
            millis() - lastInputTime >=
            INPUT_REPEAT_DELAY
        ) {
            lastInputTime =
                millis();

            return true;
        }
    }

    return false;
}

bool Menu::navigationDown()
{
    float value =
        readNavigation();

    if (value >= 0.15f) {
        if (
            millis() - lastInputTime >=
            INPUT_REPEAT_DELAY
        ) {
            lastInputTime =
                millis();

            return true;
        }
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
    if (selected) {
        tft.fillRoundRect(
            8,
            y,
            304,
            38,
            4,
            TFT_DARKCYAN
        );
    }
    else {
        tft.fillRoundRect(
            8,
            y,
            304,
            38,
            4,
            TFT_BLACK
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
        58,
        selectedMainItem ==
        MainItem::BODY_HEIGHT
    );

    tft.setTextColor(
        TFT_WHITE,
        selectedMainItem ==
        MainItem::BODY_HEIGHT
        ? TFT_DARKCYAN
        : TFT_BLACK
    );

    tft.setTextSize(1);

    tft.setCursor(
        22,
        69
    );

    tft.print(
        "Body Height"
    );

    char heightText[32];

    snprintf(
        heightText,
        sizeof(heightText),
        "%.0f mm",
        bodyHeight
    );

    int heightWidth =
        strlen(heightText) * 12;

    tft.setCursor(
        300 - heightWidth,
        69
    );

    tft.print(
        heightText
    );

    drawSelection(
        106,
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
        117
    );

    tft.print(
        "Gait"
    );

    const char* gaitName;

    switch (selectedGait) {
        case GaitItem::TRIPOD:
            gaitName = "Tripod";
            break;

        case GaitItem::WAVE:
            gaitName = "Wave";
            break;

        case GaitItem::DANCE:
            gaitName = "Dance";
            break;

        default:
            gaitName = "Tripod";
            break;
    }

    int gaitWidth =
        strlen(gaitName) * 12;

    tft.setCursor(
        300 - gaitWidth,
        117
    );

    tft.print(
        gaitName
    );

    drawFooter(
        "B1 BACK",
        "B2 SELECT"
    );
}

void Menu::drawBodyHeightMenu()
{
    tft.fillScreen(
        TFT_BLACK
    );

    drawHeader(
        "BODY HEIGHT"
    );

    tft.setTextColor(
        TFT_LIGHTGREY,
        TFT_BLACK
    );

    tft.setTextSize(1);

    tft.setCursor(
        125,
        68
    );

    tft.print(
        "Height"
    );

    drawBodyHeightValue();

    tft.setTextSize(1);

    tft.setTextColor(
        TFT_LIGHTGREY,
        TFT_BLACK
    );

    tft.setCursor(
        134,
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

void Menu::drawBodyHeightValue()
{
    tft.fillRect(
        55,
        92,
        210,
        48,
        TFT_BLACK
    );

    char valueText[32];

    snprintf(
        valueText,
        sizeof(valueText),
        "%.0f mm",
        bodyHeight
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

void Menu::drawGaitMenu()
{
    tft.fillScreen(
        TFT_BLACK
    );

    drawHeader(
        "GAIT"
    );

    drawSelection(
        48,
        selectedGait ==
        GaitItem::TRIPOD
    );

    tft.setTextColor(
        TFT_WHITE,
        selectedGait ==
        GaitItem::TRIPOD
        ? TFT_DARKCYAN
        : TFT_BLACK
    );

    tft.setTextSize(1);

    tft.setCursor(
        22,
        59
    );

    tft.print(
        "Tripod"
    );

    drawSelection(
        94,
        selectedGait ==
        GaitItem::WAVE
    );

    tft.setTextColor(
        TFT_WHITE,
        selectedGait ==
        GaitItem::WAVE
        ? TFT_DARKCYAN
        : TFT_BLACK
    );

    tft.setCursor(
        22,
        105
    );

    tft.print(
        "Wave"
    );

    drawSelection(
        140,
        selectedGait ==
        GaitItem::DANCE
    );

    tft.setTextColor(
        TFT_WHITE,
        selectedGait ==
        GaitItem::DANCE
        ? TFT_DARKCYAN
        : TFT_BLACK
    );

    tft.setCursor(
        22,
        151
    );

    tft.print(
        "Dance"
    );

    drawFooter(
        "B1 BACK",
        "B2 SELECT"
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

    if (state == State::CAMERA) {
        if (button1) {
            enterMenu();

            return true;
        }

        return false;
    }

    if (state == State::MAIN) {
        if (button1) {
            leaveMenu();

            return false;
        }

        if (navigationUp()) {
            selectedMainItem =
                MainItem::BODY_HEIGHT;

            drawMainMenu();
        }

        if (navigationDown()) {
            selectedMainItem =
                MainItem::GAIT;

            drawMainMenu();
        }

        if (button2) {
            if (
                selectedMainItem ==
                MainItem::BODY_HEIGHT
            ) {
                state =
                    State::BODY_HEIGHT;

                drawBodyHeightMenu();
            }
            else {
                state =
                    State::GAIT;

                drawGaitMenu();
            }
        }

        return true;
    }

    if (state == State::BODY_HEIGHT) {
        if (button1 || button2) {
            state =
                State::MAIN;

            drawMainMenu();

            return true;
        }

        float navigation =
            readNavigation();

        if (
            navigation > 0.15f ||
            navigation < -0.15f
        ) {
            if (
                millis() - lastInputTime >=
                INPUT_REPEAT_DELAY
            ) {
                lastInputTime =
                    millis();

                if (navigation > 0.15f) {
                    adjustBodyHeight(1.0f);
                }
                else {
                    adjustBodyHeight(-1.0f);
                }
            }
        }

        return true;
    }

    if (state == State::GAIT) {
        if (button1) {
            state =
                State::MAIN;

            drawMainMenu();

            return true;
        }

        if (navigationUp()) {
            if (
                selectedGait ==
                GaitItem::WAVE
            ) {
                selectedGait =
                    GaitItem::TRIPOD;
            }
            else if (
                selectedGait ==
                GaitItem::DANCE
            ) {
                selectedGait =
                    GaitItem::WAVE;
            }

            drawGaitMenu();
        }

        if (navigationDown()) {
            if (
                selectedGait ==
                GaitItem::TRIPOD
            ) {
                selectedGait =
                    GaitItem::WAVE;
            }
            else if (
                selectedGait ==
                GaitItem::WAVE
            ) {
                selectedGait =
                    GaitItem::DANCE;
            }

            drawGaitMenu();
        }

        if (button2) {
            RobotProtocol::GaitTypeWire gait;

            switch (selectedGait) {
                case GaitItem::TRIPOD:
                    gait =
                        RobotProtocol::GaitTypeWire::TRIPOD;
                    break;

                case GaitItem::WAVE:
                    gait =
                        RobotProtocol::GaitTypeWire::WAVE;
                    break;

                case GaitItem::DANCE:
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

            state =
                State::MAIN;

            drawMainMenu();
        }

        return true;
    }

    return true;
}