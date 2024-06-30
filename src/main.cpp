#include "Sim808.h"
#include "GpsLocationInfo.h"

#include <Arduino.h>

Sim808 gsm(Serial3);

void setup()
{
    Serial.begin(115200);
    Serial3.begin(9600);

    if (!gsm.enableGps(true))
    {
        Serial.println(F("GPS cannot be enabled."));
    }
}

void loop()
{
    gsm.printGpsLocationInfo(gsm.getGpsLocationInfo(), Serial);
    delay(3000);
}
