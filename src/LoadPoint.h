//
// Created by neonll on 23.01.2021.
//

#ifndef ESP32_ZPB30A1_REMOTE_LOADPOINT_H
#define ESP32_ZPB30A1_REMOTE_LOADPOINT_H


#include <WString.h>
#include <HardwareSerial.h>

class LoadPoint {

public:
    unsigned long t;
    bool isActive;
    char reg;
    int temp;
    unsigned int v12;
    unsigned int vL;
    unsigned int vS;
    unsigned int iSet;
    unsigned long mWs;
    unsigned long mAs;

    void fill(const String& str);
    void printSerial(HardwareSerial &serial) const;

};


#endif //ESP32_ZPB30A1_REMOTE_LOADPOINT_H
