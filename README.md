# ESP32_ZPB30A1_Remote
Remote control and logger for ZPB30A1 electronic DC load.

## Features
* Storing logs in offline and online mode
* Wi-Fi connection
* Remote control via browser
* Setting all available preferences using remote control
* Realtime logs operating
* Export logs as image or csv/xlsx

## Prerequisites
This repository uses majestic [ESPAsync_WiFiManager](https://github.com/khoih-prog/ESPAsync_WiFiManager). 

Also you need my fork of [ZPB30A1 Firmware](https://github.com/neonll/ZPB30A1_Firmware) that implements another log format and several new remote commands.

## Hardware
* You may use either ESP32 Dev Board or make your own board that fits original device without any additional wires.
* If you want to make such board you may use files from `hardware` dir.
  ![board view](https://raw.githubusercontent.com/neonll/ESP32_ZPB30A1_Remote/master/hardware/esp32_remote_board.png)

## Installation
Just clone this repository in PlatformIO.
After ESP32 flashing you can visit http://your-esp32-ip/edit `(user: admin, password: admin)` to upload files stored in `/upload` folder.

## Some notes
* Web interface is in russian. Please ask if you need it in english.
* Old log files can be deleted using ESP32 Web Server file manager `/edit`.
* Communication with ZPB30A1 uses 16 and 17 pins of ESP32. Connection scheme and circuit board diagram will be published soon.

## Contributing
If you want to contribute to this project:

* Report bugs and errors
* Ask for enhancements
* Create issues and pull requests
* Tell other people about this library

## License and credits
* This project is licensed under GPL-3.0 License

## Copyright
Copyright 2021 - neonll