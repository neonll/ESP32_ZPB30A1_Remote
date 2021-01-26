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
    Serial.println("GETS");

}

void LoadSettings::printSerial(HardwareSerial &serial) const {
    serial.print("Timestamp: ");
    serial.println(this->t);

    serial.print("Mode: ");
    serial.println(this->mode);

    serial.print("Setpoint: ");
    serial.println(this->setpoint);

    serial.print("Beeper: ");
    serial.println(this->beeper_enabled ? "Enabled" : "Disabled");

    serial.print("Cutoff: ");
    serial.println(this->cutoff_enabled ? "Enabled" : "Disabled");

    serial.print("Cutoff voltage: ");
    serial.println(this->cutoff_voltage);

    serial.print("Current Limit: ");
    serial.println(this->current_limit);

    serial.print("Power limit: ");
    serial.println(this->max_power_action ? "Enabled" : "Disabled");

}

