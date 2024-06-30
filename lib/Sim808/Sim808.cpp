#include "Sim808.h"

#include "Regexp.h"

#include "GpsLocationInfo.h"
#include "UtcTime.h"

namespace
{
    constexpr auto COMMAND_LENGTH = 15;
    constexpr auto RESPONSE_LENGTH = 110;
    constexpr auto RESPONSE_TIMEOUT_MS = 3000;
    constexpr auto UPDATE_TIMEOUT_MS = 100;
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

Sim808::Sim808(Stream &stream)
    : m_stream{&stream},
      m_response{new char[RESPONSE_LENGTH]},
      m_currentResponseIndex{0}
{
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

bool Sim808::enableGps(const bool enable)
{
    char command[COMMAND_LENGTH];
    sprintf(command, "AT+CGPSPWR=%d\n", enable ? 1 : 0);
    return sendCommandExpectingOkResponse(command);
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
    return sendCommandExpectingOkResponse(command);
}

GpsLocationInfo Sim808::getGpsLocationInfo()
{
    GpsLocationInfo locationInfo;
    char command[COMMAND_LENGTH];
    strcpy(command, "AT+CGPSINF=0\n");

    if (sendCommandExpectingOkResponse(command))
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

bool Sim808::sendCommandExpectingOkResponse(char *msg)
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

        if (strstr(m_response, "OK") != nullptr)
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