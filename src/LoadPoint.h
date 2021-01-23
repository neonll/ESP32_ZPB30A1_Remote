//
// Created by neonll on 23.01.2021.
//

#ifndef ESP32_ZPB30A1_REMOTE_LOADPOINT_H
#define ESP32_ZPB30A1_REMOTE_LOADPOINT_H


#include <WString.h>
#include <HardwareSerial.h>

class LoadPoint {

private:
    bool isActive;
    char reg;
    int temp;
    unsigned int v12;
    unsigned int vL;
    unsigned int vS;
    unsigned int iSet;
    unsigned long mWs;
    unsigned long mAs;

    static String getValue(const String& data, char separator, int index);

public:
    void fill(const String& str);

    bool getIsActive() const;
    char getReg() const;
    int getTemp() const;
    unsigned int getV12() const;
    unsigned int getVL() const;
    unsigned int getVS() const;
    unsigned int getISet() const;
    unsigned int getMWs() const;
    unsigned int getMAs() const;

    void printSerial(HardwareSerial &serial) const;

};


#endif //ESP32_ZPB30A1_REMOTE_LOADPOINT_H
