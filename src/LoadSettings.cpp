//
// Created by neonll on 26.01.2021.
//

#include "LoadSettings.h"
#include "ParseString.h"
#include "Timestamp.h"

void LoadSettings::fill(const String &str) {
    this->t = timestamp();
    this->mode = ParseString::getValue(str, ',', 0);
    this->setpoint = ParseString::getValue(str, ',', 1).toInt();
    this->beeper_enabled = ParseString::getValue(str, ',', 2) == "true";
    this->cutoff_enabled = ParseString::getValue(str, ',', 3) == "true";
    this->cutoff_voltage = ParseString::getValue(str, ',', 4).toInt();
    this->current_limit = ParseString::getValue(str, ',', 5).toInt();
    this->max_power_action = ParseString::getValue(str, ',', 6) == "true";
}

void LoadSettings::request(HardwareSerial &Serial) {
    Serial.write("GETS");

}
