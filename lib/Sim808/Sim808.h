#pragma once

#include "Arduino.h"

struct GpsLocationInfo;

namespace Sim808Constants
{
    constexpr auto INVALID_PIN = 255;
}

class Sim808
{
public:
    enum class GpsResetMode
    {
        Cold,
        Hot,
        Warm
    };

    enum class GpsStatus
    {
        Unknown,
        NotFix,
        Fix2D,
        Fix3D
    };

    Sim808(Stream &stream,
           uint8_t resetPin = Sim808Constants::INVALID_PIN,
           uint8_t powerPin = Sim808Constants::INVALID_PIN,
           uint8_t statusPin = Sim808Constants::INVALID_PIN);
    ~Sim808();

    Stream *getStream();
    int available();
    int read();

    bool init();
    void reset();
    void powerCycle();
    bool isAlive();
    void keepAlive();

    // GSM
    bool setSmsTextMode();
    bool sendSms(char *phoneNumber, char *message);

    // GPS
    bool isGpsEnabled();
    bool enableGps(bool enable);
    bool setGpsResetMode(GpsResetMode resetMode);
    GpsLocationInfo getGpsLocationInfo();
    void printGpsLocationInfo(const GpsLocationInfo &locationInfo, Stream &stream);
    GpsStatus getGpsStatus();

private:
    Stream *m_stream;
    char *m_response;
    uint8_t m_currentResponseIndex;
    uint8_t m_resetPin;
    uint8_t m_powerPin;
    uint8_t m_statusPin;

    bool sendCommandExpectingResponse(char *msg, const char *response = "OK");
    void resetResponse();
    bool isResponseFull();
    void clearBuffer();

    /// @brief Convert GPS coordinate to decimal degrees.
    /// @param coordinate Coordinate in ddmm.mmm format
    /// @return Coordinate in dd.ddddd format
    float convertCoordinate(float coordinate);
};
