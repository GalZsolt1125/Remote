#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include "esp_heap_caps.h"
#include "RobotClient.h"

const char* ssid = "ESP32-CAM-STREAM";
const char* password = "011235813";
const char* host = "192.168.4.1";

const int STREAM_PORT = 80;
const int CONTROL_PORT = 9000;

#define ROBOT_JOY_X_PIN   1
#define ROBOT_JOY_Y_PIN   2

#define HEIGHT_JOY_Y_PIN  7
#define RIGHT_JOY_X_PIN   6

#define BUTTON_1_PIN      4
#define BUTTON_2_PIN      5

#define JOYSTICK_CENTER   2048

TFT_eSPI tft = TFT_eSPI();
WiFiClient streamClient;
RobotClient robotClient;

uint8_t* jpg_buffer = NULL;
const size_t MAX_JPG_SIZE = 64 * 1024;
uint8_t net_buf[1024];

unsigned long lastMotionSend = 0;
const unsigned long MOTION_INTERVAL = 50;

float currentBodyHeight = -20.0f;
const float MIN_HEIGHT = -100.0f;
const float MAX_HEIGHT = 25.0f;

bool isMoving = false;

bool tft_output(
    int16_t x,
    int16_t y,
    uint16_t w,
    uint16_t h,
    uint16_t* bitmap
)
{
    if (y >= tft.height()) {
        return false;
    }

    tft.pushImage(x, y, w, h, bitmap);

    return true;
}

float readJoystick(int pin, bool invert)
{
    int value = analogRead(pin);
    float result;

    if (value > JOYSTICK_CENTER) {
        result =
            (float)(value - JOYSTICK_CENTER) /
            (4095.0f - JOYSTICK_CENTER);
    } else {
        result =
            (float)(value - JOYSTICK_CENTER) /
            JOYSTICK_CENTER;
    }

    if (fabs(result) < 0.10f) {
        result = 0.0f;
    }

    if (invert) {
        result = -result;
    }

    return constrain(result, -1.0f, 1.0f);
}

void updateRobotControl()
{
    float y =
        readJoystick(
            ROBOT_JOY_Y_PIN,
            true
        );

    float x =
        readJoystick(
            RIGHT_JOY_X_PIN,
            true
        );

    float raw_rot =
        readJoystick(
            ROBOT_JOY_X_PIN,
            true
        );

    float rot = raw_rot * 10.0f;

    if (y > 0.05f) {
        rot = -rot;
    }

    if (
        fabs(x) > 0.01f ||
        fabs(y) > 0.01f ||
        fabs(rot) > 0.01f
    )
    {
        robotClient.setMotion(
            x,
            y,
            rot
        );

        isMoving = true;
    }
    else if (isMoving)
    {
        robotClient.setMotion(
            0.0f,
            0.0f,
            0.0f
        );

        isMoving = false;
    }

    float heightInput =
        readJoystick(
            HEIGHT_JOY_Y_PIN,
            false
        );

    if (fabs(heightInput) > 0.1f) {
        currentBodyHeight += heightInput * 8.0f;

        currentBodyHeight =
            constrain(
                currentBodyHeight,
                MIN_HEIGHT,
                MAX_HEIGHT
            );

        robotClient.setBodyHeight(
            currentBodyHeight
        );
    }
}

void connectStream()
{
    if (streamClient.connected()) {
        return;
    }

    if (streamClient.connect(host, STREAM_PORT)) {
        streamClient.println("GET /stream HTTP/1.1");
        streamClient.print("Host: ");
        streamClient.println(host);
        streamClient.println("Connection: close");
        streamClient.println();
    }
}

void updateStream()
{
    if (!streamClient.connected()) {
        return;
    }

    static size_t jpg_pos = 0;
    static bool in_jpg = false;
    static uint8_t prev_byte = 0;

    while (streamClient.available() > 0) {
        int bytesRead =
            streamClient.read(
                net_buf,
                sizeof(net_buf)
            );

        if (bytesRead <= 0) {
            break;
        }

        for (int i = 0; i < bytesRead; i++) {
            uint8_t b = net_buf[i];

            if (
                !in_jpg &&
                prev_byte == 0xFF &&
                b == 0xD8
            )
            {
                in_jpg = true;

                jpg_buffer[0] = 0xFF;
                jpg_buffer[1] = 0xD8;

                jpg_pos = 2;
            }
            else if (in_jpg)
            {
                if (jpg_pos < MAX_JPG_SIZE) {
                    jpg_buffer[jpg_pos++] = b;
                }

                if (
                    prev_byte == 0xFF &&
                    b == 0xD9
                )
                {
                    in_jpg = false;

                    if (jpg_pos > 100) {
                        TJpgDec.drawJpg(
                            0,
                            0,
                            jpg_buffer,
                            jpg_pos
                        );
                    }

                    jpg_pos = 0;

                    break;
                }
            }

            prev_byte = b;
        }
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(17, OUTPUT);
    digitalWrite(17, HIGH);

    pinMode(BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(BUTTON_2_PIN, INPUT_PULLUP);

    analogReadResolution(12);

    delay(1000);

    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(
        "Inditas...",
        10,
        10,
        2
    );

    if (psramFound()) {
        jpg_buffer =
            (uint8_t*)heap_caps_malloc(
                MAX_JPG_SIZE,
                MALLOC_CAP_SPIRAM |
                MALLOC_CAP_8BIT
            );
    }
    else {
        jpg_buffer =
            (uint8_t*)malloc(
                MAX_JPG_SIZE
            );
    }

    TJpgDec.setJpgScale(1);
    TJpgDec.setCallback(tft_output);
    TJpgDec.setSwapBytes(true);

    WiFi.begin(
        ssid,
        password
    );

    while (WiFi.status() != WL_CONNECTED) {
        delay(200);
    }

    tft.fillScreen(TFT_BLACK);

    robotClient.begin(
        host,
        CONTROL_PORT
    );
}

void loop()
{
    if (!jpg_buffer) {
        return;
    }

    connectStream();
    updateStream();

    if (!robotClient.connected()) {
        if (robotClient.connect()) {
            robotClient.setBodyHeight(
                currentBodyHeight
            );
        }
    }

    if (
        robotClient.connected() &&
        millis() - lastMotionSend >= MOTION_INTERVAL
    )
    {
        lastMotionSend = millis();

        updateRobotControl();
    }

    delay(1);
}