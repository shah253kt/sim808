#include "Sim808.h"
#include "GpsLocationInfo.h"

#include <Arduino.h>

Sim808 gsm(Serial3);

constexpr auto POWER = 2;
constexpr auto RESET = 3;
constexpr auto STATUS = 4;

void setup()
{
    Serial.begin(115200);
    Serial3.begin(9600);

    Serial.println("Resetting...");

    pinMode(RESET, OUTPUT);
    pinMode(POWER, OUTPUT);
    pinMode(STATUS, INPUT);

    gsm.reset(RESET);
    gsm.powerCycle(POWER);

    delay(3000);

    Serial.println("Initializing...");
    const auto beginInitializationAt = millis();
    auto initialized = false;

    while (millis() - beginInitializationAt < 3000)
    {
        initialized = gsm.init();
    }

    if (!initialized)
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
    if (Serial.available())
    {
        Serial3.write(Serial.read());
        // char c = Serial.read();

        // switch (c)
        // {
        // case 's':
        // {
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
        gsm.printGpsLocationInfo(gsm.getGpsLocationInfo(), Serial);
        auto gpsStatus = gsm.getGpsStatus();
        Serial.print(F("GPS status: "));
        Serial.println(static_cast<int>(gpsStatus));
        lastGpsRequestedAt = millis();
    }
}
