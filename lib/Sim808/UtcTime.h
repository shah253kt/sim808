#pragma once

#include "Arduino.h"

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
    uint8_t day{1};
    uint8_t month{1};
    uint16_t year{2024};
    uint8_t hour{0};
    uint8_t minute{0};
    uint8_t second{0};

    void parse(char *utcTime)
    {
        if (strlen(utcTime) != strlen(UTC_TIME_FORMAT))
        {
            return;
        }

        char buffer[YEAR_LENGTH + 1];
        strncpy(buffer, utcTime + YEAR_INDEX, YEAR_LENGTH);
        buffer[YEAR_LENGTH] = '\0';
        year = atoi(buffer);
        strncpy(buffer, utcTime + MONTH_INDEX, MONTH_LENGTH);
        buffer[MONTH_LENGTH] = '\0';
        month = atoi(buffer);
        strncpy(buffer, utcTime + DAY_INDEX, DAY_LENGTH);
        buffer[DAY_LENGTH] = '\0';
        day = atoi(buffer);
        strncpy(buffer, utcTime + HOUR_INDEX, HOUR_LENGTH);
        buffer[HOUR_LENGTH] = '\0';
        hour = atoi(buffer);
        strncpy(buffer, utcTime + MINUTE_INDEX, MINUTE_LENGTH);
        buffer[MINUTE_LENGTH] = '\0';
        minute = atoi(buffer);
        strncpy(buffer, utcTime + SECOND_INDEX, SECOND_LENGTH);
        buffer[SECOND_LENGTH] = '\0';
        second = atoi(buffer);
    }

    void print(Stream &stream) const
    {
        char format[40];
        sprintf(format, "Date and time: %02d/%02d/%04d %02d:%02d:%02d", day, month, year, hour, minute, second);
        stream.println(format);
    }

    bool operator==(const UtcTime &other)
    {
        return day == other.day &&
               month == other.month &&
               year == other.year &&
               hour == other.hour &&
               minute == other.minute &&
               second == other.second;
    }
};
