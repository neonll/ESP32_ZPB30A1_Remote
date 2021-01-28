#if !defined(ESP32)
#error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.
#endif

// Use from 0 to 4. Higher number, more debugging messages and memory usage.
#define _ESPASYNC_WIFIMGR_LOGLEVEL_    3

// Default is 30s, using 20s now
#define TIME_BETWEEN_MODAL_SCANS          20000UL

// Default is 60s, using 30s now
#define TIME_BETWEEN_MODELESS_SCANS       30000UL

#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>

// From v1.1.0
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
//////

// You only need to format the filesystem once
//#define FORMAT_FILESYSTEM       true
#define FORMAT_FILESYSTEM         false

// Use LittleFS
#include "FS.h"
#include "LoadPoint.h"
#include "LoadState.h"
#include "Timestamp.h"
#include "LoadRx.h"

LoadState State;

// The library will be depreciated after being merged to future major Arduino esp32 core release 2.x
// At that time, just remove this library inclusion
#include <LITTLEFS.h>             // https://github.com/lorol/LITTLEFS

FS* filesystem =      &LITTLEFS;
#define FileFS        LITTLEFS
#define FS_Name       "LittleFS"



#define LED_BUILTIN       2
#define LED_ON            HIGH
#define LED_OFF           LOW

#include <SPIFFSEditor.h>

// SSID and PW for Config Portal
String ssid = "ESP_" + String((uint32_t)ESP.getEfuseMac(), HEX);
const char* password = "12345678";

// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

// From v1.1.0
#define MIN_AP_PASSWORD_SIZE    8

#define SSID_MAX_LEN            32
//From v1.0.10, WPA2 passwords can be up to 63 characters long.
#define PASS_MAX_LEN            64

typedef struct
{
    char wifi_ssid[SSID_MAX_LEN];
    char wifi_pw  [PASS_MAX_LEN];
}  WiFi_Credentials;

typedef struct
{
    String wifi_ssid;
    String wifi_pw;
}  WiFi_Credentials_String;

#define NUM_WIFI_CREDENTIALS      2

typedef struct
{
    WiFi_Credentials  WiFi_Creds [NUM_WIFI_CREDENTIALS];
} WM_Config;

WM_Config         WM_config;

#define  CONFIG_FILENAME              F("/wifi_cred.dat")
//////

// Indicates whether ESP has WiFi credentials saved from previous session, or double reset detected
bool initialConfig = false;

// Use false if you don't like to display Available Pages in Information Page of Config Portal
// Comment out or use true to display Available Pages in Information Page of Config Portal
// Must be placed before #include <ESP_WiFiManager.h>
#define USE_AVAILABLE_PAGES     false

// From v1.0.10 to permit disable/enable StaticIP configuration in Config Portal from sketch. Valid only if DHCP is used.
// You'll loose the feature of dynamically changing from DHCP to static IP, or vice versa
// You have to explicitly specify false to disable the feature.
//#define USE_STATIC_IP_CONFIG_IN_CP          false

// Use false to disable NTP config. Advisable when using Cellphone, Tablet to access Config Portal.
// See Issue 23: On Android phone ConfigPortal is unresponsive (https://github.com/khoih-prog/ESP_WiFiManager/issues/23)
#define USE_ESP_WIFIMANAGER_NTP     false

// Use true to enable CloudFlare NTP service. System can hang if you don't have Internet access while accessing CloudFlare
// See Issue #21: CloudFlare link in the default portal (https://github.com/khoih-prog/ESP_WiFiManager/issues/21)
#define USE_CLOUDFLARE_NTP          false

#define USING_CORS_FEATURE          true
//////

// Use USE_DHCP_IP == true for dynamic DHCP IP, false to use static IP which you have to change accordingly to your network
#if (defined(USE_STATIC_IP_CONFIG_IN_CP) && !USE_STATIC_IP_CONFIG_IN_CP)
// Force DHCP to be true
  #if defined(USE_DHCP_IP)
    #undef USE_DHCP_IP
  #endif
  #define USE_DHCP_IP     true
#else
// You can select DHCP or Static IP here
#define USE_DHCP_IP     true
// #define USE_DHCP_IP     false
#endif

#if ( USE_DHCP_IP || ( defined(USE_STATIC_IP_CONFIG_IN_CP) && !USE_STATIC_IP_CONFIG_IN_CP ) )
// Use DHCP
#warning Using DHCP IP
IPAddress stationIP   = IPAddress(0, 0, 0, 0);
IPAddress gatewayIP   = IPAddress(192, 168, 1, 1);
IPAddress netMask     = IPAddress(255, 255, 255, 0);
#else
// Use static IP
  #warning Using static IP

  IPAddress stationIP   = IPAddress(192, 168, 1, 232);
  IPAddress gatewayIP   = IPAddress(192, 168, 1, 1);
  IPAddress netMask     = IPAddress(255, 255, 255, 0);
#endif

#define USE_CONFIGURABLE_DNS      false

IPAddress dns1IP      = gatewayIP;
IPAddress dns2IP      = IPAddress(8, 8, 8, 8);

IPAddress APStaticIP  = IPAddress(192, 168, 100, 1);
IPAddress APStaticGW  = IPAddress(192, 168, 100, 1);
IPAddress APStaticSN  = IPAddress(255, 255, 255, 0);

#include <ESPAsync_WiFiManager.h>              //https://github.com/khoih-prog/ESPAsync_WiFiManager
#include <AsyncJson.h>

//#define ARDUINOJSON_EMBEDDED_MODE 1
#include "ArduinoJson.h"
#include "ParseString.h"

String host = "async-esp32fs";

#define HTTP_PORT     80

AsyncWebServer server(HTTP_PORT);
DNSServer dnsServer;

AsyncEventSource events("/events");

String http_username = "admin";
String http_password = "admin";

String separatorLine = "===============================================================";

///////////////////////////////////////////
// New in v1.4.0
/******************************************
 * // Defined in ESPAsync_WiFiManager.h
typedef struct
{
  IPAddress _ap_static_ip;
  IPAddress _ap_static_gw;
  IPAddress _ap_static_sn;

}  WiFi_AP_IPConfig;

typedef struct
{
  IPAddress _sta_static_ip;
  IPAddress _sta_static_gw;
  IPAddress _sta_static_sn;
#if USE_CONFIGURABLE_DNS
  IPAddress _sta_static_dns1;
  IPAddress _sta_static_dns2;
#endif
}  WiFi_STA_IPConfig;
******************************************/

WiFi_AP_IPConfig  WM_AP_IPconfig;
WiFi_STA_IPConfig WM_STA_IPconfig;

void initAPIPConfigStruct(WiFi_AP_IPConfig &in_WM_AP_IPconfig)
{
    in_WM_AP_IPconfig._ap_static_ip   = APStaticIP;
    in_WM_AP_IPconfig._ap_static_gw   = APStaticGW;
    in_WM_AP_IPconfig._ap_static_sn   = APStaticSN;
}

void initSTAIPConfigStruct(WiFi_STA_IPConfig &in_WM_STA_IPconfig)
{
    in_WM_STA_IPconfig._sta_static_ip   = stationIP;
    in_WM_STA_IPconfig._sta_static_gw   = gatewayIP;
    in_WM_STA_IPconfig._sta_static_sn   = netMask;
#if USE_CONFIGURABLE_DNS
    in_WM_STA_IPconfig._sta_static_dns1 = dns1IP;
  in_WM_STA_IPconfig._sta_static_dns2 = dns2IP;
#endif
}

void displayIPConfigStruct(WiFi_STA_IPConfig in_WM_STA_IPconfig)
{
    LOGERROR3(F("stationIP ="), in_WM_STA_IPconfig._sta_static_ip, ", gatewayIP =", in_WM_STA_IPconfig._sta_static_gw);
    LOGERROR1(F("netMask ="), in_WM_STA_IPconfig._sta_static_sn);
#if USE_CONFIGURABLE_DNS
    LOGERROR3(F("dns1IP ="), in_WM_STA_IPconfig._sta_static_dns1, ", dns2IP =", in_WM_STA_IPconfig._sta_static_dns2);
#endif
}

void configWiFi(WiFi_STA_IPConfig in_WM_STA_IPconfig)
{
#if USE_CONFIGURABLE_DNS
    // Set static IP, Gateway, Subnetmask, DNS1 and DNS2. New in v1.0.5
    WiFi.config(in_WM_STA_IPconfig._sta_static_ip, in_WM_STA_IPconfig._sta_static_gw, in_WM_STA_IPconfig._sta_static_sn, in_WM_STA_IPconfig._sta_static_dns1, in_WM_STA_IPconfig._sta_static_dns2);
#else
    // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
    WiFi.config(in_WM_STA_IPconfig._sta_static_ip, in_WM_STA_IPconfig._sta_static_gw, in_WM_STA_IPconfig._sta_static_sn);
#endif
}

///////////////////////////////////////////

uint8_t connectMultiWiFi()
{
// For ESP32, this better be 0 to shorten the connect time
#define WIFI_MULTI_1ST_CONNECT_WAITING_MS       0

#define WIFI_MULTI_CONNECT_WAITING_MS           100L

    uint8_t status;

    LOGERROR(F("ConnectMultiWiFi with :"));

    if ( (Router_SSID != "") && (Router_Pass != "") )
    {
        LOGERROR3(F("* Flash-stored Router_SSID = "), Router_SSID, F(", Router_Pass = "), Router_Pass );
    }

    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
        // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
        if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
        {
            LOGERROR3(F("* Additional SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
        }
    }

    LOGERROR(F("Connecting MultiWifi..."));

    WiFi.mode(WIFI_STA);

#if !USE_DHCP_IP
    // New in v1.4.0
  configWiFi(WM_STA_IPconfig);
  //////
#endif

    int i = 0;
    status = wifiMulti.run();
    delay(WIFI_MULTI_1ST_CONNECT_WAITING_MS);

    while ( ( i++ < 20 ) && ( status != WL_CONNECTED ) )
    {
        status = wifiMulti.run();

        if ( status == WL_CONNECTED )
            break;
        else
            delay(WIFI_MULTI_CONNECT_WAITING_MS);
    }

    if ( status == WL_CONNECTED )
    {
        LOGERROR1(F("WiFi connected after time: "), i);
        LOGERROR3(F("SSID:"), WiFi.SSID(), F(",RSSI="), WiFi.RSSI());
        LOGERROR3(F("Channel:"), WiFi.channel(), F(",IP address:"), WiFi.localIP() );
    }
    else
        LOGERROR(F("WiFi not connected"));

    return status;
}


//format bytes
String formatBytes(size_t bytes)
{
    if (bytes < 1024)
    {
        return String(bytes) + "B";
    }
    else if (bytes < (1024 * 1024))
    {
        return String(bytes / 1024.0) + "KB";
    }
    else if (bytes < (1024 * 1024 * 1024))
    {
        return String(bytes / 1024.0 / 1024.0) + "MB";
    }
    else
    {
        return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
    }
}

void toggleLED()
{
    //toggle state
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void heartBeatPrint()
{
    static int num = 1;

    if (WiFi.status() == WL_CONNECTED)
        Serial.print(F("H"));        // H means connected to WiFi
    else
        Serial.print(F("F"));        // F means not connected to WiFi

    if (num == 80)
    {
        Serial.println();
        num = 1;
    }
    else if (num++ % 10 == 0)
    {
        Serial.print(F(" "));
    }
}

void check_WiFi()
{
    if ( (WiFi.status() != WL_CONNECTED) )
    {
        Serial.println(F("\nWiFi lost. Call connectMultiWiFi in loop"));
        connectMultiWiFi();
    }
}

void check_status()
{
    static ulong checkstatus_timeout  = 0;
    static ulong LEDstatus_timeout    = 0;
    static ulong checkwifi_timeout    = 0;

    static ulong current_millis;

#define WIFICHECK_INTERVAL    1000L
#define LED_INTERVAL          2000L
#define HEARTBEAT_INTERVAL    10000L

    current_millis = millis();

    // Check WiFi every WIFICHECK_INTERVAL (1) seconds.
    if ((current_millis > checkwifi_timeout) || (checkwifi_timeout == 0))
    {
        check_WiFi();
        checkwifi_timeout = current_millis + WIFICHECK_INTERVAL;
    }

    if ((current_millis > LEDstatus_timeout) || (LEDstatus_timeout == 0))
    {
        // Toggle LED at LED_INTERVAL = 2s
        toggleLED();
        LEDstatus_timeout = current_millis + LED_INTERVAL;
    }

    // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
    if ((current_millis > checkstatus_timeout) || (checkstatus_timeout == 0))
    {
        heartBeatPrint();
        checkstatus_timeout = current_millis + HEARTBEAT_INTERVAL;
    }
}

bool loadConfigData()
{
    File file = FileFS.open(CONFIG_FILENAME, "r");
    LOGERROR(F("LoadWiFiCfgFile "));

    memset(&WM_config,       0, sizeof(WM_config));

    // New in v1.4.0
    memset(&WM_STA_IPconfig, 0, sizeof(WM_STA_IPconfig));
    //////

    if (file)
    {
        file.readBytes((char *) &WM_config,   sizeof(WM_config));

        // New in v1.4.0
        file.readBytes((char *) &WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
        //////

        file.close();
        LOGERROR(F("OK"));

        // New in v1.4.0
        displayIPConfigStruct(WM_STA_IPconfig);
        //////

        return true;
    }
    else
    {
        LOGERROR(F("failed"));

        return false;
    }
}

void saveConfigData()
{
    File file = FileFS.open(CONFIG_FILENAME, "w");
    LOGERROR(F("SaveWiFiCfgFile "));

    if (file)
    {
        file.write((uint8_t*) &WM_config,   sizeof(WM_config));

        // New in v1.4.0
        file.write((uint8_t*) &WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
        //////

        file.close();
        LOGERROR(F("OK"));
    }
    else
    {
        LOGERROR(F("failed"));
    }
}

void listFilesInDir(File dir, int numTabs = 1);

String listLogFiles();


void listFilesInDir(File dir, int numTabs) {
    while (true) {

        File entry =  dir.openNextFile();
        if (! entry) {
            // no more files in the folder
            break;
        }
        for (uint8_t i = 0; i < numTabs; i++) {
            Serial.print('\t');
        }
        Serial.print(entry.name());
        if (entry.isDirectory()) {
            Serial.println("/");
            listFilesInDir(entry, numTabs + 1);
        } else {
            // display zise for file, nothing for directory
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}

String listLogFiles() {
    File dir = FileFS.open("/");

    String list;

    while (true) {

        File entry =  dir.openNextFile();
        if (! entry) {
            // no more files in the folder
            break;
        }

        if (ParseString::getValue(entry.name(), '.', 1) == "csv") {
//            list[i] = entry.name();
//            i++;
//            Serial.println(entry.name());
            list += entry.name();
            list += ",";
        }

        entry.close();
    }

    return list;
}


void setup()
{
    //set led pin as output
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    while (!Serial);

    Serial.print("\nStarting Async_ESP32_FSWebServer using " + String(FS_Name));
    Serial.println(" on " + String(ARDUINO_BOARD));
    Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION);

    Serial.setDebugOutput(false);

    if (FORMAT_FILESYSTEM)
        FileFS.format();

    // Format SPIFFS if not yet
    if (!FileFS.begin(true))
    {
        Serial.print(FS_Name);
        Serial.println(F(" failed! AutoFormatting."));
    }

    File root = FileFS.open("/");
    File file = root.openNextFile();

    while (file)
    {
        String fileName = file.name();
        size_t fileSize = file.size();
        Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
        file = root.openNextFile();
    }

    Serial.println();

    unsigned long startedAt = millis();

    // New in v1.4.0
    initAPIPConfigStruct(WM_AP_IPconfig);
    initSTAIPConfigStruct(WM_STA_IPconfig);
    //////

    digitalWrite(LED_BUILTIN, LED_ON);

    //Local intialization. Once its business is done, there is no need to keep it around
    // Use this to default DHCP hostname to ESP8266-XXXXXX or ESP32-XXXXXX
    //ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer);
    // Use this to personalize DHCP hostname (RFC952 conformed)
    ESPAsync_WiFiManager ESPAsync_wifiManager(&server, &dnsServer, "AsyncESP32-FSWebServer");

    //set custom ip for portal
    // New in v1.4.0
    ESPAsync_wifiManager.setAPStaticIPConfig(WM_AP_IPconfig);
    //////

    ESPAsync_wifiManager.setMinimumSignalQuality(-1);

    // Set config portal channel, default = 1. Use 0 => random channel from 1-13
    ESPAsync_wifiManager.setConfigPortalChannel(0);
    //////

#if !USE_DHCP_IP
    // Set (static IP, Gateway, Subnetmask, DNS1 and DNS2) or (IP, Gateway, Subnetmask). New in v1.0.5
    // New in v1.4.0
    ESPAsync_wifiManager.setSTAStaticIPConfig(WM_STA_IPconfig);
    //////
#endif

    // New from v1.1.1
#if USING_CORS_FEATURE
    // ESPAsync_wifiManager.setCORSHeader("Your Access-Control-Allow-Origin");
#endif

    // We can't use WiFi.SSID() in ESP32as it's only valid after connected.
    // SSID and Password stored in ESP32 wifi_ap_record_t and wifi_config_t are also cleared in reboot
    // Have to create a new function to store in EEPROM/SPIFFS for this purpose
    Router_SSID = ESPAsync_wifiManager.WiFi_SSID();
    Router_Pass = ESPAsync_wifiManager.WiFi_Pass();

    //Remove this line if you do not want to see WiFi password printed
    Serial.println("Stored: SSID = " + Router_SSID + ", Pass = " + Router_Pass);

    // SSID to uppercase
    ssid.toUpperCase();

    bool configDataLoaded = loadConfigData();

    if (!configDataLoaded)
    {
        // From v1.1.0, Don't permit NULL password
        if ( (Router_SSID == "") || (Router_Pass == "") )
        {
            Serial.println(F("We haven't got any access point credentials, so get them now"));

            initialConfig = true;

            // Starts an access point
            if (!ESPAsync_wifiManager.startConfigPortal((const char *) ssid.c_str(), password))
                Serial.println(F("Not connected to WiFi but continuing anyway."));
            else
                Serial.println(F("WiFi connected...yeey :)"));

            // Stored  for later usage, from v1.1.0, but clear first
            memset(&WM_config, 0, sizeof(WM_config));

            for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
            {
                String tempSSID = ESPAsync_wifiManager.getSSID(i);
                String tempPW   = ESPAsync_wifiManager.getPW(i);

                if (strlen(tempSSID.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1)
                    strcpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str());
                else
                    strncpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1);

                if (strlen(tempPW.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1)
                    strcpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str());
                else
                    strncpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1);

                // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
                if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
                {
                    LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
                    wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
                }
            }

            // New in v1.4.0
            ESPAsync_wifiManager.getSTAStaticIPConfig(WM_STA_IPconfig);
            displayIPConfigStruct(WM_STA_IPconfig);
            //////

            saveConfigData();
        }
        else
        {
            wifiMulti.addAP(Router_SSID.c_str(), Router_Pass.c_str());
        }
    }

    startedAt = millis();

    if (!initialConfig)
    {
        // Load stored data, the addAP ready for MultiWiFi reconnection
        if (!configDataLoaded)
            loadConfigData();

        for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
        {
            // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
            if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
            {
                LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
                wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
            }
        }

        if ( WiFi.status() != WL_CONNECTED )
        {
            Serial.println(F("ConnectMultiWiFi in setup"));

            connectMultiWiFi();
        }
    }

    Serial.print(F("After waiting "));
    Serial.print((float) (millis() - startedAt) / 1000L);
    Serial.print(F(" secs more in setup(), connection result is "));

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print(F("connected. Local IP: "));
        Serial.println(WiFi.localIP());
    }
    else
        Serial.println(ESPAsync_wifiManager.getStatus(WiFi.status()));

    if ( !MDNS.begin(host.c_str()) )
    {
        Serial.println(F("Error starting MDNS responder!"));
    }

    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", HTTP_PORT);

    //SERVER INIT
    events.onConnect([](AsyncEventSourceClient * client)
                     {
                         client->send("hello!", NULL, millis(), 1000);
                     });

    server.addHandler(&events);

    server.on("/heap", HTTP_GET, [](AsyncWebServerRequest * request)
    {
        request->send(200, "text/plain", String(ESP.getFreeHeap()));
    });

    server.addHandler(new SPIFFSEditor(FileFS, http_username, http_password));
    server.serveStatic("/", FileFS, "/").setDefaultFile("index.html");

    server.on("/getState", HTTP_GET, [](AsyncWebServerRequest *request){
        StaticJsonDocument<1024> doc;

        JsonObject settings = doc.createNestedObject("settings");
        settings["t"] = State.settings.t;
        settings["mode"] = State.settings.mode;
        settings["setpoint"] = State.settings.setpoint;
        settings["beeper_enabled"] = State.settings.beeper_enabled;
        settings["cutoff_enabled"] = State.settings.cutoff_enabled;
        settings["cutoff_voltage"] = State.settings.cutoff_voltage;
        settings["current_limit"] = State.settings.current_limit;
        settings["max_power_action"] = State.settings.max_power_action;

        JsonObject point = doc.createNestedObject("point");
        point["t"] = State.point.t;
        point["isActive"] = State.point.isActive;
        point["reg"] = State.point.reg;
        point["temp"] = State.point.temp;
        point["v12"] = State.point.v12;
        point["vL"] = State.point.vL;
        point["vS"] = State.point.vS;
        point["iSet"] = State.point.iSet;
        point["mWs"] = State.point.mWs;
        point["mAs"] = State.point.mAs;


        String output;
        serializeJson(doc, output);

        request->send(200, "application/json", output);
    });

    server.on("/getLogs", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", listLogFiles());
    });

    server.on("/loadOn", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial1.println("LOAD ON");
        Serial1.println("LOAD ON");
        Serial1.println("LOAD ON");
        Serial1.println("LOAD ON");
        request->send(200, "text/plain", "OK");
    });

    server.on("/loadOff", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial1.println("LOAD OFF");
        request->send(200, "text/plain", "OK");
    });

    server.on("/setMode", HTTP_GET, [](AsyncWebServerRequest *request){

        AsyncWebParameter* p = request->getParam("mode");

        Serial1.print("SETMODE ");
        Serial1.println(p->value());

        request->send(200, "text/plain", p->value());
    });

    server.on("/setVal", HTTP_GET, [](AsyncWebServerRequest *request){

        AsyncWebParameter* type = request->getParam("type");
        AsyncWebParameter* val = request->getParam("val");

        Serial1.print("SET"+type->value()+" ");
        Serial1.println(val->value());

        request->send(200, "text/plain", val->value());
    });

    server.on("/setCutoff", HTTP_GET, [](AsyncWebServerRequest *request){

        AsyncWebParameter* val = request->getParam("val");

        Serial1.print("SETCUTOFF ");
        Serial1.println(val->value());

        request->send(200, "text/plain", val->value());
    });

    server.on("/setBeep", HTTP_GET, [](AsyncWebServerRequest *request){

        AsyncWebParameter* val = request->getParam("val");

        Serial1.print("SETBEEP ");
        Serial1.println(val->value());

        request->send(200, "text/plain", val->value());
    });

    server.on("/setPLimit", HTTP_GET, [](AsyncWebServerRequest *request){

        AsyncWebParameter* val = request->getParam("val");

        Serial1.print("SETPLIMIT ");
        Serial1.println(val->value());

        request->send(200, "text/plain", val->value());
    });

        server.onNotFound([](AsyncWebServerRequest * request)
                      {
                          Serial.print(F("NOT_FOUND: "));

                          if (request->method() == HTTP_GET)
                              Serial.print(F("GET"));
                          else if (request->method() == HTTP_POST)
                              Serial.print(F("POST"));
                          else if (request->method() == HTTP_DELETE)
                              Serial.print(F("DELETE"));
                          else if (request->method() == HTTP_PUT)
                              Serial.print(F("PUT"));
                          else if (request->method() == HTTP_PATCH)
                              Serial.print(F("PATCH"));
                          else if (request->method() == HTTP_HEAD)
                              Serial.print(F("HEAD"));
                          else if (request->method() == HTTP_OPTIONS)
                              Serial.print(F("OPTIONS"));
                          else
                              Serial.print(F("UNKNOWN"));

                          Serial.println(" http://" + request->host() + request->url());

                          if (request->contentLength())
                          {
                              Serial.println("_CONTENT_TYPE: " + request->contentType());
                              Serial.println("_CONTENT_LENGTH: " + request->contentLength());
                          }

                          int headers = request->headers();
                          int i;

                          for (i = 0; i < headers; i++)
                          {
                              AsyncWebHeader* h = request->getHeader(i);
                              Serial.println("_HEADER[" + h->name() + "]: " + h->value());
                          }

                          int params = request->params();

                          for (i = 0; i < params; i++)
                          {
                              AsyncWebParameter* p = request->getParam(i);

                              if (p->isFile())
                              {
                                  Serial.println("_FILE[" + p->name() + "]: " + p->value() + ", size: " + p->size());
                              }
                              else if (p->isPost())
                              {
                                  Serial.println("_POST[" + p->name() + "]: " + p->value());
                              }
                              else
                              {
                                  Serial.println("_GET[" + p->name() + "]: " + p->value());
                              }
                          }

                          request->send(404);
                      });

    server.onFileUpload([](AsyncWebServerRequest * request, const String & filename, size_t index, uint8_t *data, size_t len, bool final)
                        {
                            (void) request;

                            if (!index)
                                Serial.println("UploadStart: " + filename);

                            Serial.print((const char*)data);

                            if (final)
                                Serial.println("UploadEnd: " + filename + "(" + String(index + len) + ")" );
                        });

    server.onRequestBody([](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
                         {
                             (void) request;

                             if (!index)
                                 Serial.println("BodyStart: " + total);

                             Serial.print((const char*)data);

                             if (index + len == total)
                                 Serial.println("BodyEnd: " + total);
                         });

    server.begin();

    //////

    Serial.print(F("HTTP server started @ "));
    Serial.println(WiFi.localIP());

    Serial.println(separatorLine);
    Serial.println("Open http://" + host + ".local/edit to see the file browser");
    Serial.println("Using username = " + http_username + " and password = " + http_password);
    Serial.println(separatorLine);

    // Connecting to load

    #define RXD1 16 // RX1 pin
    #define TXD1 17 //TX2 pin

    Serial1.begin(115200, SERIAL_8N1, RXD1, TXD1);

    Serial.println("Load connected!");
    Serial.println(separatorLine);

    digitalWrite(LED_BUILTIN, LED_OFF);

    Timestamp t;
    t.init();

    Serial.println(timestamp());

//    WiFi.disconnect(true);
//    WiFi.mode(WIFI_OFF);

//    unsigned int totalBytes = FileFS.totalBytes();
//    unsigned int usedBytes = FileFS.usedBytes();
//
//    Serial.println("===== File system info =====");
//
//    Serial.print("Total space:      ");
//    Serial.print(totalBytes);
//    Serial.println("byte");
//
//    Serial.print("Total space used: ");
//    Serial.print(usedBytes);
//    Serial.println("byte");
//
//    Serial.println();

//    // Open dir folder
//    File dir = FileFS.open("/");
//    // List file at root
//    listFilesInDir(dir);


}

//String LoadRx;
byte T = 0;
unsigned long lastW = 0;
bool logStarted = false;
File file;

void logStart() {
    String filepath = "/" + String(timestamp()) + ".csv";

    file = FileFS.open(filepath, "w");
//    LOGERROR(F("SaveLogFile "));

    logStarted = true;
    Serial.println("Started new log: " + filepath);
}

void logWrite() {
    file.println(String(State.point.t) + ","
            + String(State.point.reg) + ","
            + String(State.point.temp) + ","
            + String(State.point.v12) + ","
            + String(State.point.vL) + ","
            + String(State.point.vS) + ","
            + String(State.point.iSet) + ","
            + String(State.point.mWs) + ","
            + String(State.point.mAs));

    lastW = State.point.t;
}

void logClose() {
    file.close();
    logStarted = false;
    Serial.println("Log closed.");
}

void loop()
{
    check_status();

    while (Serial1.available()) {

//        LoadRx = Serial1.readStringUntil('\n');

        LoadRx loadRx;

        loadRx.process(Serial1, State);
//        Serial.println(loadRx.getValue());

        if (State.point.isActive) {
            if (State.point.t - lastW >= 15) {
                if (logStarted) {
                    logWrite();
                }
                else {
                    logStart();
                    logWrite();
                }
            }
        }
        else if (State.point.temp > 0) {
            if (logStarted)
                logClose();
        }

        if (T == 5) {
//            Serial.print(timestamp());
//            Serial.println(": Settings");
//            State.settings.printSerial(Serial);
            T++;
        }
        if (T == 10) {
            State.settings.request(Serial1);
//            Serial.print(timestamp());
//            Serial.println(": Point");
//            State.point.printSerial(Serial);

            T = 0;
        } else {
            T++;
        }

    }
}
