#pragma once

#include "Arduino.h"

#include "TimeLib.h"

namespace
{
    constexpr char UTC_TIME_FORMAT[] = "yyyymmddHHMMSS.SSS"; // Example: 20240630115106.000: 2024/06/30 11:51:06
    constexpr auto YEAR_INDEX = 0;
    constexpr auto YEAR_LENGTH = 4;
    constexpr auto MONTH_INDEX = 4;
    constexpr auto MONTH_LENGTH = 2;
    constexpr auto DAY_INDEX = 6;
    constexpr auto DAY_LENGTH = 2;
    constexpr auto HOUR_INDEX = 8;
    constexpr auto HOUR_LENGTH = 2;
    constexpr auto MINUTE_INDEX = 10;
    constexpr auto MINUTE_LENGTH = 2;
    constexpr auto SECOND_INDEX = 12;
    constexpr auto SECOND_LENGTH = 2;
}

struct UtcTime
{
    TimeElements timeElements;

    void parse(char *utcTime)
    {
        if (strlen(utcTime) != strlen(UTC_TIME_FORMAT))
        {
            return;
        }

        char buffer[YEAR_LENGTH + 1];
        strncpy(buffer, utcTime + YEAR_INDEX, YEAR_LENGTH);
        buffer[YEAR_LENGTH] = '\0';
        timeElements.Year = atoi(buffer);
        strncpy(buffer, utcTime + MONTH_INDEX, MONTH_LENGTH);
        buffer[MONTH_LENGTH] = '\0';
        timeElements.Month = atoi(buffer);
        strncpy(buffer, utcTime + DAY_INDEX, DAY_LENGTH);
        buffer[DAY_LENGTH] = '\0';
        timeElements.Day = atoi(buffer);
        strncpy(buffer, utcTime + HOUR_INDEX, HOUR_LENGTH);
        buffer[HOUR_LENGTH] = '\0';
        timeElements.Hour = atoi(buffer);
        strncpy(buffer, utcTime + MINUTE_INDEX, MINUTE_LENGTH);
        buffer[MINUTE_LENGTH] = '\0';
        timeElements.Minute = atoi(buffer);
        strncpy(buffer, utcTime + SECOND_INDEX, SECOND_LENGTH);
        buffer[SECOND_LENGTH] = '\0';
        timeElements.Second = atoi(buffer);
    }

    void print(Stream &stream) const
    {
        char format[40];
        sprintf(format, "Date and time: %02d/%02d/%04d %02d:%02d:%02d",
                timeElements.Day,
                timeElements.Month,
                timeElements.Year,
                timeElements.Hour,
                timeElements.Minute,
                timeElements.Second);
        stream.println(format);
    }

    void getStrings(char *dateString, char *timeString, int timezonOffset = 0)
    {
        auto utc = makeTime(timeElements);
        utc += constrain(timezonOffset, -11, 12) * 3600;
        sprintf(dateString, "%02d/%02d/%04d", day(utc), month(utc), year(utc));
        sprintf(timeString, "%02d:%02d:%02d", hour(utc), minute(utc), second(utc));
    }

    UtcTime clone()
    {
        UtcTime utc;
        utc.clone(*this);
        return utc;
    }

    void clone(const UtcTime &other)
    {
        timeElements = other.timeElements;
    }

    bool operator==(const UtcTime &other)
    {
        return timeElements.Day == other.timeElements.Day &&
               timeElements.Month == other.timeElements.Month &&
               timeElements.Year == other.timeElements.Year &&
               timeElements.Hour == other.timeElements.Hour &&
               timeElements.Minute == other.timeElements.Minute &&
               timeElements.Second == other.timeElements.Second;
    }
};
