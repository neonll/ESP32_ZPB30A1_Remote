//
// Created by neonll on 26.01.2021.
//

#ifndef ESP32_ZPB30A1_REMOTE_PARSESTRING_H
#define ESP32_ZPB30A1_REMOTE_PARSESTRING_H


#include <WString.h>

class ParseString {

public:
    static String getValue(const String& data, char separator, int index);
};


#endif //ESP32_ZPB30A1_REMOTE_PARSESTRING_H
