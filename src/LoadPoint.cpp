//
// Created by neonll on 23.01.2021.
//

#include "LoadPoint.h"

String LoadPoint::getValue(const String& data, char separator, int index) {
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length()-1;

    for(int i=0; i<=maxIndex && found<=index; i++){
        if(data.charAt(i)==separator || i==maxIndex){
            found++;
            strIndex[0] = strIndex[1]+1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }

    return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void LoadPoint::fill(const String& str) {
    char regulator = getValue(str, ',', 0).charAt(0);

    this->isActive = regulator != 'N';
    this->reg = regulator;
    this->temp = getValue(str, ',', 1).toInt();
    this->v12 = getValue(str, ',', 2).toInt();
    this->vL = getValue(str, ',', 3).toInt();
    this->vS = getValue(str, ',', 4).toInt();
    this->iSet = getValue(str, ',', 5).toInt();
    this->mWs = getValue(str, ',', 6).toInt();
    this->mAs = getValue(str, ',', 7).toInt();
}

bool LoadPoint::getIsActive() const {
    return isActive;
}

char LoadPoint::getReg() const {
    return reg;
}

int LoadPoint::getTemp() const {
    return temp;
}

unsigned int LoadPoint::getV12() const {
    return v12;
}

unsigned int LoadPoint::getVL() const {
    return vL;
}

unsigned int LoadPoint::getVS() const {
    return vS;
}

unsigned int LoadPoint::getISet() const {
    return iSet;
}

unsigned int LoadPoint::getMWs() const {
    return mWs;
}

unsigned int LoadPoint::getMAs() const {
    return mAs;
}

void LoadPoint::printSerial(HardwareSerial &serial) const {
    serial.print("Active: ");
    serial.println(getIsActive() ? "Active" : "Inactive");

    serial.print("Regulated: ");
    serial.println(getReg());

    serial.print("Temp: ");
    serial.println(getTemp());

    serial.print("V supply: ");
    serial.println(getV12());

    serial.print("V load: ");
    serial.println(getVL());

    serial.print("V sensor: ");
    serial.println(getVS());

    serial.print("I set: ");
    serial.println(getISet());

    serial.print("mWs: ");
    serial.println(getMWs());

    serial.print("mAs: ");
    serial.println(getMAs());
}
