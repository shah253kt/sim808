#include "Sim808.h"

#include "Regexp.h"

#include "GpsLocationInfo.h"
#include "UtcTime.h"

namespace
{
    constexpr auto COMMAND_LENGTH = 20;
    constexpr auto RESPONSE_LENGTH = 110;
    constexpr auto RESPONSE_TIMEOUT_MS = 3000;
    constexpr auto POWER_CYCLE_INTERVAL_MS = 3000;
}

namespace Regex
{
    constexpr auto GPS_LOCATION_INFO_RESPONSE = "+CGPSINF:%s*(%d+),([^,]+),([^,]+),([^,]+),([^,]+),(%d+),(%d+),([^,]+),([^,]+)";
}

namespace ResponseIndex
{
    constexpr auto MODE = 0;
    constexpr auto LONGITUDE = 1;
    constexpr auto LATITUDE = 2;
    constexpr auto ALTITUDE = 3;
    constexpr auto UTC_TIME = 4;
    constexpr auto TIME_TO_FIRST_FIX = 5;
    constexpr auto SATELLITE_IN_VIEW = 6;
    constexpr auto SPEED = 7;
    constexpr auto COURSE = 8;
}

Sim808::Sim808(Stream &stream, uint8_t resetPin, uint8_t powerPin, uint8_t statusPin)
    : m_stream{&stream},
      m_response{new char[RESPONSE_LENGTH]},
      m_currentResponseIndex{0},
      m_resetPin{resetPin},
      m_powerPin{powerPin},
      m_statusPin{statusPin}
{
    pinMode(m_resetPin, OUTPUT);
    pinMode(m_powerPin, OUTPUT);
    pinMode(m_statusPin, INPUT);
}

Sim808::~Sim808()
{
    delete[] m_response;
}

Stream *Sim808::getStream()
{
    return m_stream;
}

int Sim808::available()
{
    return m_stream->available();
}

int Sim808::read()
{
    return m_stream->read();
}

bool Sim808::init()
{
    if (m_resetPin != Sim808Constants::INVALID_PIN && m_powerPin != Sim808Constants::INVALID_PIN && m_statusPin != Sim808Constants::INVALID_PIN)
    {
        auto tries = 0;
        digitalWrite(m_resetPin, LOW);
        delay(1000);
        digitalWrite(m_resetPin, HIGH);

        while (tries++ < 2 && digitalRead(m_statusPin) == LOW)
        {
            digitalWrite(m_powerPin, LOW);
            delay(1000);
            digitalWrite(m_powerPin, HIGH);
            delay(1000);
            digitalWrite(m_powerPin, LOW);
            delay(3000);
        }
    }

    char command[COMMAND_LENGTH];
    strcpy(command, "AT\n");
    return sendCommandExpectingResponse(command);
}

bool Sim808::isAlive()
{
    if (m_statusPin == Sim808Constants::INVALID_PIN)
    {
        return true;
    }

    return digitalRead(m_statusPin) == HIGH;
}

void Sim808::keepAlive()
{
    if (m_resetPin == Sim808Constants::INVALID_PIN || m_powerPin == Sim808Constants::INVALID_PIN || m_statusPin == Sim808Constants::INVALID_PIN)
    {
        return;
    }

    if (digitalRead(m_statusPin) == HIGH)
    {
        return;
    }

    static auto lastPowerCycleState = false;
    static auto lastPowerCycleChangedAt = millis();

    digitalWrite(m_powerPin, lastPowerCycleState);

    if (millis() - lastPowerCycleChangedAt >= POWER_CYCLE_INTERVAL_MS)
    {
        lastPowerCycleChangedAt = millis();
        lastPowerCycleState = !lastPowerCycleState;
    }
}

bool Sim808::setSmsTextMode()
{
    char command[COMMAND_LENGTH];
    strcpy(command, "AT+CMGF=1\n");
    return sendCommandExpectingResponse(command);
}

bool Sim808::sendSms(char *phoneNumber, char *message)
{
    if (!setSmsTextMode())
    {
        return false;
    }

    m_stream->print("AT+CMGS=");
    m_stream->print(phoneNumber);
    m_stream->print('\r');
    m_stream->print(message);
    m_stream->print('\x1A'); // Ctrl-Z
    return true;
}

bool Sim808::enableGps(const bool enable)
{
    char command[COMMAND_LENGTH];
    sprintf(command, "AT+CGPSPWR=%d\n", enable ? 1 : 0);
    return sendCommandExpectingResponse(command);
}

bool Sim808::setGpsResetMode(Sim808::GpsResetMode resetMode)
{
    auto resetModeValue = [resetMode]
    {
        switch (resetMode)
        {
        case Sim808::GpsResetMode::Cold:
        {
            return 0;
        }
        case Sim808::GpsResetMode::Hot:
        {
            return 1;
        }
        case Sim808::GpsResetMode::Warm:
        {
            return 2;
        }
        }

        return -1;
    }();

    if (resetModeValue == -1)
    {
        return false;
    }

    char command[COMMAND_LENGTH];
    sprintf(command, "AT+CGPSRST=%d\n", resetModeValue);
    return sendCommandExpectingResponse(command);
}

GpsLocationInfo Sim808::getGpsLocationInfo()
{
    GpsLocationInfo locationInfo;
    char command[COMMAND_LENGTH];
    strcpy(command, "AT+CGPSINF=0\n");

    if (sendCommandExpectingResponse(command))
    {
        static MatchState ms;
        ms.Target(m_response);
        char buffer[strlen(m_response)];

        if (ms.Match(Regex::GPS_LOCATION_INFO_RESPONSE) > 0)
        {
            ms.GetCapture(buffer, ResponseIndex::LONGITUDE);
            locationInfo.longitute = atof(buffer);
            ms.GetCapture(buffer, ResponseIndex::LATITUDE);
            locationInfo.latitude = atof(buffer);
            ms.GetCapture(buffer, ResponseIndex::ALTITUDE);
            locationInfo.altitude = atof(buffer);
            ms.GetCapture(buffer, ResponseIndex::UTC_TIME);
            locationInfo.utcTime.parse(buffer);
            ms.GetCapture(buffer, ResponseIndex::SATELLITE_IN_VIEW);
            locationInfo.satelliteInView = atoi(buffer);
            ms.GetCapture(buffer, ResponseIndex::SPEED);
            locationInfo.speedOverGround = atof(buffer);
            ms.GetCapture(buffer, ResponseIndex::COURSE);
            locationInfo.courseOverGround = atof(buffer);
        }

        resetResponse();
    }

    return locationInfo;
}

void Sim808::printGpsLocationInfo(const GpsLocationInfo &locationInfo, Stream &stream)
{
    stream.println(F("==============================================="));
    stream.print(F("Latitude: "));
    stream.println(locationInfo.latitude);
    stream.print(F("Longitude: "));
    stream.println(locationInfo.longitute);
    stream.print(F("Altitude: "));
    stream.println(locationInfo.altitude);
    locationInfo.utcTime.print(stream);
    stream.print(F("Satellite in view: "));
    stream.println(locationInfo.satelliteInView);
    stream.print(F("Speed: "));
    stream.println(locationInfo.speedOverGround);
    stream.print(F("Course: "));
    stream.println(locationInfo.courseOverGround);
    stream.println(F("==============================================="));
}

Sim808::GpsStatus Sim808::getGpsStatus()
{
    GpsLocationInfo locationInfo;
    char command[COMMAND_LENGTH];
    strcpy(command, "AT+CGPSSTATUS?\n");

    if (sendCommandExpectingResponse(command))
    {
        if (strstr(m_response, "Location Unknown") != nullptr)
        {
            return Sim808::GpsStatus::Unknown;
        }
        else if (strstr(m_response, "Location Not Fix") != nullptr)
        {
            return Sim808::GpsStatus::NotFix;
        }
        else if (strstr(m_response, "Location 2D Fix") != nullptr)
        {
            return Sim808::GpsStatus::Fix2D;
        }
        else if (strstr(m_response, "Location 3D Fix") != nullptr)
        {
            return Sim808::GpsStatus::Fix3D;
        }
    }

    return Sim808::GpsStatus::Unknown;
}

bool Sim808::sendCommandExpectingResponse(char *msg, const char *response)
{
    clearBuffer();
    resetResponse();
    m_stream->print(msg);

    const auto commandSentAt = millis();
    auto receivedOkResponse = false;

    while (millis() - commandSentAt < RESPONSE_TIMEOUT_MS)
    {
        if (!available())
        {
            continue;
        }

        if (isResponseFull())
        {
            m_currentResponseIndex = 0;
            break;
        }

        m_response[m_currentResponseIndex++] = read();
        m_response[m_currentResponseIndex] = '\0';

        if (strstr(m_response, response) != nullptr)
        {
            receivedOkResponse = true;
            break;
        }
    }

    clearBuffer();

    return receivedOkResponse;
}

void Sim808::resetResponse()
{
    m_currentResponseIndex = 0;
    strcpy(m_response, "");
}

bool Sim808::isResponseFull()
{
    return strlen(m_response) >= RESPONSE_LENGTH - 1;
}

void Sim808::clearBuffer()
{
    while (available())
    {
        read();
    }
}