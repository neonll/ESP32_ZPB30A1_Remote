//
// Created by neonll on 25.01.2021.
//

#include "LoadRx.h"
#include "LoadState.h"

void LoadRx::setValue(String &data) {
    this->text = data;
}

String LoadRx::getValue() {
    return this->text;
}

void LoadRx::recognise() {
    char data[10] = "";
    this->text.toCharArray(data, 10);

    if (((data[0] == 'C') || (data[0] == 'X')) && (data[2] == ',')) {
        // settings string from GETS command
        this->type = LOAD_RX_SETTINGS;
    }
    // TODO: change space to comma after flashing
    else if (((data[0] == 'A') || (data[0] == 'U') || (data[0] == 'N')) && (data[1] == ' ') && (data[3] == ',')) {
        // point string
        this->type = LOAD_RX_POINT;
    }

}

void LoadRx::fillState(LoadState &state, String &data) {
    setValue(data);
    this->recognise();

    switch (this->type) {
        case LOAD_RX_POINT:
            state.point.fill(this->text);
            break;
        case LOAD_RX_SETTINGS:
            state.settings.fill(this->text);
            break;
        default:
            // DO NOTHING
            break;
    }
}

void LoadRx::process(HardwareSerial &Serial, LoadState &state) {
    String str = Serial.readStringUntil('\n');

    fillState(state, str);
}
