#ifndef NODECONFIG_H
#define NODECONFIG_H

#include <ESPmDNS.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <SD_MMC.h>
#include <FS.h>
#include <EEPROM.h>


#define VERSION "2022-8-23-1005-SENTRY-FDM"
#define DBG_OUTPUT_PORT Serial
#define PRINTER_PORT Serial
#define RED_LED 15
#define GREEN_LED 33
#define BLUE_LED 32

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define BUFSIZE 16
#define MAX_CMD_SIZE 48


extern String pre_line;
extern String cf_ssid;
extern String cf_password;
extern String cf_node_name;

extern String pc_ipaddress;


class NodeConfig
{
public:
    NodeConfig();
};

#endif // NODECONFIG_H
