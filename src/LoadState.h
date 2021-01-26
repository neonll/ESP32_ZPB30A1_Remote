//
// Created by neonll on 26.01.2021.
//

#ifndef ESP32_ZPB30A1_REMOTE_LOADSTATE_H
#define ESP32_ZPB30A1_REMOTE_LOADSTATE_H


#include "LoadPoint.h"
#include "LoadSettings.h"

class LoadState {

public:
    LoadPoint point{};
    LoadSettings settings{};
};


#endif //ESP32_ZPB30A1_REMOTE_LOADSTATE_H
