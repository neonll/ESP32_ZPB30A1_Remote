//
// Created by neonll on 26.01.2021.
//

#include "Timestamp.h"
#include <ctime>
#include <Esp.h>


unsigned long timestamp()
{
    struct tm timeinfo;
    time_t now;

    if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        return 0;
    }
//    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
//    Serial.println(time(&now));
    return time(&now);

}

void Timestamp::init() {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}
