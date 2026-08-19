#ifndef ROBOT_CLIENT_H
#define ROBOT_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <RobotProtocol.h>

class RobotClient
{
public:
    RobotClient();

    void begin(const char* host, uint16_t port);

    bool connect();

    void disconnect();

    bool connected();

    void start();

    void stop();

    void setMotion(float x, float y, float rot);

    void setBodyHeight(float height);

private:
    void sendPacket(
        RobotProtocol::CommandType command,
        const uint8_t* payload,
        uint8_t length
    );

    WiFiClient client;

    const char* host;
    uint16_t port;
    uint8_t txSequence;
};

#endif