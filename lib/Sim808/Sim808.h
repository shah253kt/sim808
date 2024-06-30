#pragma once

#include "Arduino.h"

struct GpsLocationInfo;

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

    Sim808(Stream &stream);
    ~Sim808();

    Stream *getStream();
    int available();
    int read();

    bool init();
    void powerCycle(uint8_t pin);
    void reset(uint8_t pin);

    // GSM
    bool setSmsTextMode();
    bool sendSms(char *phoneNumber, char *message);

    // GPS
    bool enableGps(bool enable);
    bool setGpsResetMode(GpsResetMode resetMode);
    GpsLocationInfo getGpsLocationInfo();
    void printGpsLocationInfo(const GpsLocationInfo &locationInfo, Stream &stream);
    GpsStatus getGpsStatus();

private:
    Stream *m_stream;
    char *m_response;
    uint8_t m_currentResponseIndex;

    bool sendCommandExpectingResponse(char *msg, const char *response = "OK");
    void resetResponse();
    bool isResponseFull();
    void clearBuffer();
};