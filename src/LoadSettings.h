//
// Created by neonll on 26.01.2021.
//

#ifndef ESP32_ZPB30A1_REMOTE_LOADSETTINGS_H
#define ESP32_ZPB30A1_REMOTE_LOADSETTINGS_H


#include <WString.h>
#include <HardwareSerial.h>

class LoadSettings {
public:
    String mode;
    unsigned long t;
    unsigned int setpoint;
    bool beeper_enabled;
    bool cutoff_enabled;
    unsigned int cutoff_voltage;
    unsigned int current_limit;
    bool max_power_action;

    void fill(const String& str);
    void request(HardwareSerial &Serial);
};


#endif //ESP32_ZPB30A1_REMOTE_LOADSETTINGS_H
