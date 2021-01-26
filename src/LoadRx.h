//
// Created by neonll on 25.01.2021.
//

#ifndef ESP32_ZPB30A1_REMOTE_LOADRX_H
#define ESP32_ZPB30A1_REMOTE_LOADRX_H


#include <WString.h>
#include "LoadState.h"

class LoadRx {

typedef enum {
    LOAD_RX_SETTINGS,
    LOAD_RX_POINT,
    LOAD_RX_UNKNOWN
} type_t;

private:
    String text = "";
    type_t type = LOAD_RX_UNKNOWN;

    void recognise();

public:

    void setValue(String &data);
    String getValue();
    void process(LoadState &state, String &data);


};


#endif //ESP32_ZPB30A1_REMOTE_LOADRX_H
