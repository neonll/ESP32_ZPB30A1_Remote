//
// Created by neonll on 26.01.2021.
//

#ifndef ESP32_ZPB30A1_REMOTE_TIMESTAMP_H
#define ESP32_ZPB30A1_REMOTE_TIMESTAMP_H


class Timestamp {
private:
    const char* ntpServer = "pool.ntp.org";
    const long  gmtOffset_sec = 10800;
    const int   daylightOffset_sec = 0;
public:
    void init();
};

extern unsigned long timestamp();

#endif //ESP32_ZPB30A1_REMOTE_TIMESTAMP_H
