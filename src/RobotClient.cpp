#include "RobotClient.h"

using namespace RobotProtocol;

RobotClient::RobotClient()
{
    host = nullptr;
    port = 0;
    txSequence = 0;
}

void RobotClient::begin(const char* robotHost, uint16_t robotPort)
{
    host = robotHost;
    port = robotPort;
}

bool RobotClient::connect()
{
    if (client.connected()) {
        return false;
    }

    if (host == nullptr || port == 0) {
        return false;
    }

    Serial.println("Robot control csatlakozas...");

    if (!client.connect(host, port)) {
        return false;
    }

    Serial.println("Robot control csatlakoztatva!");

    start();

    return true;
}

void RobotClient::disconnect()
{
    if (client.connected()) {
        client.stop();
    }
}

bool RobotClient::connected()
{
    return client.connected();
}

void RobotClient::sendPacket(
    CommandType command,
    const uint8_t* payload,
    uint8_t length
)
{
    if (!client.connected()) {
        return;
    }

    if (length > MAX_PAYLOAD_SIZE) {
        return;
    }

    uint8_t packet[4 + MAX_PAYLOAD_SIZE + 1];

    packet[0] = FRAME_HEADER;
    packet[1] = static_cast<uint8_t>(command);
    packet[2] = txSequence++;
    packet[3] = length;

    for (uint8_t i = 0; i < length; i++) {
        packet[4 + i] = payload[i];
    }

    uint8_t crcData[3 + MAX_PAYLOAD_SIZE];

    crcData[0] = static_cast<uint8_t>(command);
    crcData[1] = packet[2];
    crcData[2] = length;

    for (uint8_t i = 0; i < length; i++) {
        crcData[3 + i] = payload[i];
    }

    packet[4 + length] = crc8(
        crcData,
        3 + length
    );

    client.write(
        packet,
        5 + length
    );
}

void RobotClient::start()
{
    sendPacket(
        CommandType::START,
        nullptr,
        0
    );
}

void RobotClient::stop()
{
    sendPacket(
        CommandType::STOP,
        nullptr,
        0
    );
}

void RobotClient::setMotion(
    float x,
    float y,
    float rot
)
{
    MotionPayload payload;

    payload.x = x;
    payload.y = y;
    payload.rot = rot;

    sendPacket(
        CommandType::SET_MOTION,
        reinterpret_cast<const uint8_t*>(&payload),
        sizeof(payload)
    );
}

void RobotClient::setBodyHeight(float height)
{
    FloatPayload payload;

    payload.value = height;

    sendPacket(
        CommandType::SET_BODY_HEIGHT,
        reinterpret_cast<const uint8_t*>(&payload),
        sizeof(payload)
    );
}

void RobotClient::setBodyOffset(
    float x,
    float y,
    float z
)
{
    BodyOffsetPayload payload;

    payload.x = x;
    payload.y = y;
    payload.z = z;

    sendPacket(
        CommandType::SET_BODY_OFFSET,
        reinterpret_cast<const uint8_t*>(&payload),
        sizeof(payload)
    );
}

void RobotClient::setGait(GaitTypeWire gait)
{
    GaitPayload payload;

    payload.type = static_cast<uint8_t>(gait);

    sendPacket(
        CommandType::SET_GAIT,
        reinterpret_cast<const uint8_t*>(&payload),
        sizeof(payload)
    );
}

void RobotClient::setStepLength(float length)
{
    FloatPayload payload;

    payload.value = length;

    sendPacket(
        CommandType::SET_STEP_LENGTH,
        reinterpret_cast<const uint8_t*>(&payload),
        sizeof(payload)
    );
}

void RobotClient::setStepHeight(float height)
{
    FloatPayload payload;

    payload.value = height;

    sendPacket(
        CommandType::SET_STEP_HEIGHT,
        reinterpret_cast<const uint8_t*>(&payload),
        sizeof(payload)
    );
}

void RobotClient::setStepPeriod(float period)
{
    FloatPayload payload;

    payload.value = period;

    sendPacket(
        CommandType::SET_STEP_PERIOD,
        reinterpret_cast<const uint8_t*>(&payload),
        sizeof(payload)
    );
}