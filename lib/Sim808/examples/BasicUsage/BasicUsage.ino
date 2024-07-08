#include "Sim808.h"
#include "GpsLocationInfo.h"

constexpr auto POWER = 2;
constexpr auto RESET = 3;
constexpr auto STATUS = 4;

Sim808 gsm(Serial3, RESET, POWER, STATUS);

void setup()
{
    Serial.begin(115200);
    Serial3.begin(9600);

    Serial.println("Initializing...");

    if (!gsm.init())
    {
        Serial.println(F("Failed to initialize GSM"));
    }

    if (!gsm.enableGps(true))
    {
        Serial.println(F("Failed to enable GPS."));
    }

    Serial.println("Ready");
}

void loop()
{
    gsm.sendSms("+60123456789", "Hello!");
    static auto lastGpsRequestedAt = millis();

    if (millis() - lastGpsRequestedAt >= 3000)
    {
        const auto gpsPowerEnabled = gsm.isGpsEnabled();
        Serial.print(F("Is GPS power enabled? "));
        Serial.println(gpsPowerEnabled ? "Yes" : "No");

        if (!gpsPowerEnabled)
        {
            Serial.println(F("Enabling GPS"));
            gsm.enableGps(true);
        }

        auto gpsStatus = gsm.getGpsStatus();
        Serial.print(F("GPS status: "));
        switch (gpsStatus)
        {
        case Sim808::GpsStatus::Unknown:
        {
            Serial.println(F("Unknown"));
            break;
        }
        case Sim808::GpsStatus::NotFix:
        {
            Serial.println(F("Not fix"));
            break;
        }
        case Sim808::GpsStatus::Fix2D:
        {
            Serial.println(F("Fix 2D"));
            break;
        }
        case Sim808::GpsStatus::Fix3D:
        {
            Serial.println(F("Fix 3D"));
            break;
        }
        }

        gsm.printGpsLocationInfo(gsm.getGpsLocationInfo(), Serial);
        lastGpsRequestedAt = millis();
    }
}
