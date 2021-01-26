//
// Created by neonll on 23.01.2021.
//

#include "LoadPoint.h"
#include "ParseString.h"
#include "Timestamp.h"


void LoadPoint::fill(const String& str) {
    this->t = timestamp();

    char regulator = ParseString::getValue(str, ',', 0).charAt(0);

    this->isActive = regulator != 'N';
    this->reg = regulator;
    this->temp = ParseString::getValue(str, ',', 1).toInt();
    this->v12 = ParseString::getValue(str, ',', 2).toInt();
    this->vL = ParseString::getValue(str, ',', 3).toInt();
    this->vS = ParseString::getValue(str, ',', 4).toInt();
    this->iSet = ParseString::getValue(str, ',', 5).toInt();
    this->mWs = ParseString::getValue(str, ',', 6).toInt();
    this->mAs = ParseString::getValue(str, ',', 7).toInt();
}

void LoadPoint::printSerial(HardwareSerial &serial) const {
    serial.print("Active: ");
    serial.println(this->isActive ? "Active" : "Inactive");

    serial.print("Regulated: ");
    serial.println(this->reg);

    serial.print("Temp: ");
    serial.println(this->temp);

    serial.print("V supply: ");
    serial.println(this->vS);

    serial.print("V load: ");
    serial.println(this->vL);

    serial.print("V sensor: ");
    serial.println(this->vS);

    serial.print("I set: ");
    serial.println(this->iSet);

    serial.print("mWs: ");
    serial.println(this->mWs);

    serial.print("mAs: ");
    serial.println(this->mAs);
}
