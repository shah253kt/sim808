#pragma once

#include "UtcTime.h"

struct GpsLocationInfo
{
    float latitude{0.0};
    float longitute{0.0};
    float altitude{0.0};
    UtcTime utcTime;
    uint16_t satelliteInView{0};
    float speedOverGround{0.0};
    float courseOverGround{0.0};

    void clone(const GpsLocationInfo &other)
    {
        latitude = other.latitude;
        longitute = other.longitute;
        altitude = other.altitude;
        utcTime = other.utcTime;
        satelliteInView = other.satelliteInView;
        speedOverGround = other.speedOverGround;
        courseOverGround = other.courseOverGround;
    }

    bool operator==(const GpsLocationInfo &other)
    {
        return latitude == other.latitude &&
               longitute == other.longitute &&
               altitude == other.altitude &&
               utcTime == other.utcTime &&
               satelliteInView == other.satelliteInView &&
               speedOverGround == other.speedOverGround &&
               courseOverGround == other.courseOverGround;
    }
};
