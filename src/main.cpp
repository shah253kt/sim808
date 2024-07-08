#include "Sim808.h"
#include "GpsLocationInfo.h"

#include <Arduino.h>

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
    // gsm.keepAlive();

    if (Serial.available())
    {
        Serial3.write(Serial.read());
        // char c = Serial.read();

        // switch (c)
        // {
        // case 's':
        // {
        //     if (!gsm.isAlive())
        //     {
        //         break;
        //     }

        //     gsm.sendSms("+60123456789", "Hello!");
        //     break;
        // }
        // }
    }

    if (Serial3.available())
    {
        Serial.write(Serial3.read());
    }

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

        // auto gpsStatus = gsm.getGpsStatus();
        // Serial.print(F("GPS status: "));
        // Serial.println(static_cast<int>(gpsStatus));
        gsm.printGpsLocationInfo(gsm.getGpsLocationInfo(), Serial);
        lastGpsRequestedAt = millis();
        Serial.println("Done sequence");
    }
}
