#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <WebSocketsServer.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include <WiFiClientSecure.h> // thư viện gửi dữ liệu lên google sheet

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define DHTPIN D4
#define DHTTYPE DHT11
#define OLED_RESET  -1
#define SCREEN_ADDRESS 0x3C
#define button D8
#define relay D0
#define redLed D7
#define blueLed D5
#define greenLed D6
#define flashButton D3

//SoftwareSerial SIM800(RX, TX); //(RX, TX) -->SIM800A(ST,SR)
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WiFiClientSecure client; //--> Create a WiFiClientSecure object.
String GAS_ID = "AKfycbyZm-HqzncLSz71uCf-gy9mZeV25obucUuwROyZ1JuOeIi41jBhDzug72ITGsSGX4wU"; //--> spreadsheet script ID
const char* host = "script.google.com";
const int httpsPort = 443;

// Configure wifi access point
String ssidAP = "E-SMART";
String passAP = "12345678";
String cdhdAP = "1";
// Configure station
String ssidST ="ssidST";
String passST ="passST";
String cdhdST ="cdhdST";
// Configure sim800a
String sdtSend="sdtSend";
String smsSend="smsSend";
String minValueAlert="minValueAlert";
String maxValueAlert="maxValueAlert";
String cdhdAlert="cdhdAlert";
String cdhdSim800 = "cdhdSim800";
// Configure led
String lowValueLed="20.0";
String highValueLed="40.0";
// Configure relay
String lowValueRelay="lowValueRelay";
String highValueRelay="highValueRelay";
String cdhdRL="cdhdRL";

String bufferSIM800 = "";
int IndexRxdata = -1;
long count=0;
int modeState = 1;
long timeDht=0,timeGGS=0,timeReConWF=0,timeReAlert=0;
bool butPress=false;
float h=0,t=0;

IPAddress ipAP(192,168,1,1);
IPAddress gatewayAP(192,168,1,1);
IPAddress subnetAP(255,255,255,0);
WebSocketsServer webSocket = WebSocketsServer(81);
boolean ledConnect;

//Image
const unsigned char humidityImage [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x01, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x03, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x03, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x07, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x06, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x0c, 0x7f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x0c, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x08, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x18, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x18, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x0c, 0x03, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x0c, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x06, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x07, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x03, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char tempImage [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x01, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x01, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x6a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x01, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x01, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x01, 0x6b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x01, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x02, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x06, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x04, 0xf9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x05, 0xf9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x04, 0xf9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x04, 0xf3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x01, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


//==============WEB SERVER=========//
ESP8266WebServer webServer(80);
const char MainPage[] PROGMEM = R"=====(
  <!DOCTYPE html> 
  <html>
   <head> 
       <title>SMART SYSTEM CONFIGURATION</title> 
       <style> 
          body {text-align:center;background-color:#FFFFFF; color:black}
          input {height:25px; width:270px; font-size:20px;margin: 10px auto;}
          #cdhdwifiAP input {height:25px; width:25px;font-size:15px;margin: 10px 10px;}
          #cdhdwifiST input {height:25px; width:25px;font-size:15px;margin: 10px 10px;}
          #content {border: black solid 1px; padding:5px; width:330px; border-radius:20px;margin: 0 auto;}//height:380px
          #ledConnect{outline: none;margin: 10px 5px -1px 5px;width: 15px;height: 15px;border: solid 1px #222222;background-color: #00EE00;border-radius: 50%;-moz-border-radius: 50%;-webkit-border-radius: 50%;}
          .button_setup {height:30px; width:280px;  margin: 5px 0;border: solid 1px black;background-color:#FFFFFF;border-radius:5px;outline:none;color:black;font-size:15px;}
          .button_wifi{ height:50px; width:90px; margin:5px 0;outline:none;color:black;font-size:15px;font-weight: bold;}
          #wifiSetupAP{height:410px; font-size:20px; display:none;}
          #wifiSetupST{height:410px; font-size:20px; display:none;}
          #button_save {background-color:#00BB00;border-radius:5px;}
          #button_restart {background-color:#FF9900;border-radius:5px;}
          #button_reset {background-color:#CC3300;border-radius:5px;}
       </style>
       <meta name="viewport" content="width=device-width,user-scalable=0" charset="UTF-8">
   </head>
   <body> 
      <div id="content"> 

              
        <div id="homeControl" style="height:410px; display: block">
          <div style="font-size: 30px; font-weight: bold; margin:80px 25px">SYSTEM CONFIGURATION</div>
          <div>
            <button class="button_setup" onclick="configureWifiAP()">CONFIGURE WIFI ACCESS POINT</button>
            <button class="button_setup" onclick="configureWifiST()">CONFIGURE WIFI STATION</button>
            <button class="button_setup" onclick="configureSim800a()">CONFIGURE SIM800A</button>
            <button class="button_setup" onclick="configureLed()">CONFIGURE LED</button>
            <button class="button_setup" onclick="configureRelay()">CONFIGURE RELAY</button>
          </div>
        </div>

                
        <div id="wifiSetupAP" style="display: none">
          <div style="font-size: 30px; font-weight: bold">CONFIGURE WIFI ACCESS POINT</div>
          <div style="text-align:left; width:270px; margin:0px 25px">SSID Access point: </div>
          <div><input id="ssidAP"/></div>
          <div style="text-align:left; width:270px; margin:0px 25px">Password: </div>
          <div><input id="passAP"/></div>
          <div style="text-align:left;width:270px; margin:0px 25px">Operation Mode:</div>
          <div style="text-align:left; height: 50px; margin-left: 20px" id="cdhdwifiAP">
            <div style="width: 130px; float: left"><input name="cdhdwifiAP" type="radio" value="1" checked/>Show</div>     
            <div style="width: 130px; float: left"><input name="cdhdwifiAP" type="radio" value="0"/>Hide</div>
          </div>
          <div>
            <button id="button_save" class="button_wifi" onclick="writeEEPROMap()">SAVE</button>
            <button id="button_restart" class="button_wifi" onclick="restartESP()">RESTART</button>
            <button id="button_reset" class="button_wifi" onclick="clearEEPROM()">RESET</button>
          </div>
          <div>
            <input class="button_setup" type="button" onclick="backHOME()" value="BACK HOME"/>
          </div>
        </div>
        
        
        <div id="wifiSetupST" style="display: none">
          <div style="font-size: 30px; font-weight: bold; margin:20px 25px">CONFIGURE WIFI STATION</div>
          <div style="text-align:left; width:270px; margin:0px 25px">SSID STATION: </div>
          <div><input id="ssidST"/></div>
          <div style="text-align:left; width:270px; margin:0px 25px">Password: </div>
          <div><input id="passST"/></div>
          <div style="text-align:left;width:270px; margin:0px 25px">Operation Mode:</div>
          <div style="text-align:left; height: 50px; margin-left: 20px" id="cdhdwifiST">
            <div style="width: 130px; float: left"><input name="cdhdwifiST" type="radio" value="1" checked/>Enable</div>     
            <div style="width: 130px; float: left"><input name="cdhdwifiST" type="radio" value="0"/>Disable</div>
          </div>
          <div>
            <button id="button_save" class="button_wifi" onclick="writeEEPROMst()">SAVE</button>
            <button id="button_restart" class="button_wifi" onclick="restartESP()">RESTART</button>
            <button id="button_reset" class="button_wifi" onclick="clearEEPROM()">RESET</button>
          </div>
          <div>
            <input class="button_setup" type="button" onclick="backHOME()" value="BACK HOME"/>
          </div>
        </div>
        
        
        <div id="sim800aSetup" style="height: 500px; display: none">
          <div style="font-size: 30px; font-weight: bold; margin:10px 0px">CONFIGURE SIM800A</div>
          <div>
            <div style="text-align:left; width:270px; margin:0px 25px">Phone number: </div>
            <div><input id="sdtSend" type="tel" maxlength='10' /></div>
            <div style="text-align:left; width:270px; margin:0px 25px">Content of the message sent: (<pan id="count"></pan>)</div>
            <div style="width:270px; margin:0px 25px"><textarea id="smsSend" rows="5" cols="34" maxlength='160'></textarea></div>
          </div>
          <div>
            <div style="text-align:left; width:90px; margin:0px 25px">Min Value: </div>
            <div><input id="minValueAlert" type="tel" maxlength='05' style="width:80px;text-align:center; margin:-10px 25px"></div>
            <div style="text-align:left; width:90px; margin:0px 25px">Max Value: </div>
            <div><input id="maxValueAlert" type="tel" maxlength='05' style="width:80px;text-align:center; margin:-10px 25px"></div>
          </div>
          <div style="width: 270px; height: 50px; margin:10px 30px">
            <div style="text-align: left;">Operation Mode:</div>
            <div style="width: 130px;float: left; margin:-10px -15px"><input style="height: 25px; width: 25px" type="radio" name="cdhdAlert" checked="checked" value="0">Enable</div>
            <div style="width: 130px;float: left; margin:-15px 20px"><input style="height: 25px; width: 25px; margin:15px 0px" type="radio" name="cdhdAlert" value="1">Disable</div>
            <div style="width: 130px;float: left"><input style="height: 25px; width: 25px" type="radio" name="cdhdSim800a" checked="checked" value="0">Call Mobile</div>
            <div style="width: 130px;float: left"><input style="height: 25px; width: 25px" type="radio" name="cdhdSim800a" value="1">Send SMS</div>
          </div>
          <div>
            <button style="background-color: #00BB00; height: 50px; width: 135px;border-radius: 5px;color: black;font-weight: bold;margin-right: 5px" onclick="writeEEPROMsim800a()">SAVE CONFIG</button>
            <button style="background-color: blue; height: 50px; width: 135px;border-radius: 5px;color: black;font-weight: bold" onclick="test_sim800a()">TEST SIM800A</button>
          </div>
          <div>
            <input class="button_setup" type="button" onclick="backHOME()" value="BACK HOME"/>
          </div>
        </div>
        
        
        <div id="ledSetup" style="height: 410px; display: none">
          <div style="font-size: 30px; font-weight: bold; margin:20px 25px">CONFIGURE LED</div>
          <div style="text-align:center; width:290px; margin:10px 25px">Low value-Blue light-Mid value-Green light-High value-Red light</div>
          <div>
            <div style="text-align:left; width:90px; margin:0px 25px">Low value: </div>
            <div><input id="lowValueLed" type="tel" maxlength='05' style="width:80px;text-align:center"></div>
            <div style="text-align:left; width:90px; margin:0px 25px">High value: </div>
            <div><input id="highValueLed" type="tel" maxlength='05' style="width:80px;text-align:center"></div>
          </div>
          <div>
            <button style="background-color: #00BB00; height: 50px; width: 135px;border-radius: 5px;color: black;font-weight: bold;margin-right: 5px" onclick="writeEEPROMled()">SAVE CONFIG</button>
            <button style="background-color: blue; height: 50px; width: 135px;border-radius: 5px;color: black;font-weight: bold" onclick="test_led()">TEST LED</button>
          </div>
          <div>
            <input class="button_setup" type="button" onclick="backHOME()" value="BACK HOME"/>
          </div>
        </div>
        
        
        <div id="relaySetup" style="height: 410px; display: none">
          <div style="font-size: 30px; font-weight: bold; margin:20px 25px">CONFIGURE RELAY</div>
          <div>
            <div style="text-align:left; width:90px; margin:0px 25px">Low value: </div>
            <div><input id="lowValueRelay" type="tel" maxlength='05' style="width:80px;text-align:center"></div>
            <div style="text-align:left; width:90px; margin:0px 25px">High value: </div>
            <div><input id="highValueRelay" type="tel" maxlength='05' style="width:80px;text-align:center"></div>
          </div>
          <div style="width: 270px; height: 50px; margin-left: 30px">
            <div style="text-align: left;">Operation Mode:</div>
            <div style="width: 130px;float: left"><input style="height: 25px; width: 25px;" type="radio" name="cdhdRelay" checked="checked" value="0">Manual</div>
            <div style="width: 130px;float: left"><input style="height: 25px; width: 25px" type="radio" name="cdhdRelay" value="1">Auto</div>
          </div>
          <div>
            <button style="background-color: #00BB00; height: 50px; width: 135px;border-radius: 5px;color: black;font-weight: bold;margin-right: 5px" onclick="writeEEPROMrelay()">SAVE CONFIG</button>
            <button style="background-color: blue; height: 50px; width: 135px;border-radius: 5px;color: black;font-weight: bold" onclick="test_relay()">TEST RELAY</button>
          </div>
          <div>
            <input class="button_setup" type="button" onclick="backHOME()" value="BACK HOME"/>
          </div>
        </div>
        
      </div>
      
      
      <div id="footer">
        <p><i>gdbf <b>sdfgd</b> 
          <br>dfgdf <b>dfgdfg</b></i>
        </p>
      </div>
      
      
      <script>
      
        window.onload = function(){init();}
        
        //-----------Hàm khởi tạo đối tượng request----------------
        function create_obj(){
          td = navigator.appName;
          if(td == "Microsoft Internet Explorer"){
            obj = new ActiveXObject("Microsoft.XMLHTTP");
          }else{
            obj = new XMLHttpRequest();
          }
          return obj;
        }

        var xhttp = create_obj();
        
        //===========Configure WiFi ACCESS POINT=====================================
        function configureWifiAP(){
          document.getElementById("homeControl").style.display = "none";
          document.getElementById("wifiSetupAP").style.display = "block";
          document.getElementById("ssidAP").value = "E-SMART";
          document.getElementById("passAP").value = "12345678";
        }
        
        //===========Configure WiFi STATION=====================================
        function configureWifiST(){
          document.getElementById("homeControl").style.display = "none";
          document.getElementById("wifiSetupST").style.display = "block";
          document.getElementById("ssidST").value = "YourSSIDWifi";
          document.getElementById("passST").value = "YourPassword";
        }
        
        //-----------Confugure SIM800A---------------------------------------------
        function configureSim800a(){
          document.getElementById("homeControl").style.display = "none";
          document.getElementById("sim800aSetup").style.display = "block";
          document.getElementById("sdtSend").value = "0123456789";
          document.getElementById("smsSend").value = "Alert message";
          document.getElementById("minValueAlert").value = "30.0";
          document.getElementById("maxValueAlert").value = "40.0";
        }
        
        //===========Configure LED=====================================
        var xhttp = create_obj();
        function configureLed(){
          document.getElementById("homeControl").style.display = "none";
          document.getElementById("ledSetup").style.display = "block";
          document.getElementById("lowValueLed").value = "20.0";
          document.getElementById("highValueLed").value = "40.0";
        }
        
        //===========Configure RELAY=====================================
        var xhttp = create_obj();
        function configureRelay(){
          document.getElementById("homeControl").style.display = "none";
          document.getElementById("relaySetup").style.display = "block";
          document.getElementById("lowValueRelay").value = "20.0";
          document.getElementById("highValueRelay").value = "40.0";
        }
                
        //--------Back Home control-------------------------
        function backHOME(){
          document.getElementById("homeControl").style.display = "block";
          document.getElementById("wifiSetupAP").style.display = "none";
          document.getElementById("wifiSetupST").style.display = "none";
          document.getElementById("sim800aSetup").style.display = "none";
          document.getElementById("ledSetup").style.display = "none";
          document.getElementById("relaySetup").style.display = "none";
        }
        
        //-----------Write configure Wifi Access Point------------------------------------
        function writeEEPROMap(){
          if(Empty(document.getElementById("ssidAP"), "Please enter ssid!")&&Empty(document.getElementById("passAP"), "Please enter password")){
            var ssidAP = document.getElementById("ssidAP").value;
            var passAP = document.getElementById("passAP").value;
            var cdhdwifiAP = document.getElementsByName("cdhdwifiAP");
            for (var i = 0, length = cdhdwifiAP.length; i < length; i++) {
                if (cdhdwifiAP[i].checked) {
                    cdhdwifiAP=cdhdwifiAP[i].value;    
                    break;
                }
            }
            xhttp.open("GET","/writeEEPROMap?ssidAP="+ssidAP+"&passAP="+passAP+"&cdhdwifiAP="+cdhdwifiAP,true);
            xhttp.onreadystatechange = process;//nhận reponse 
            xhttp.send();
          }
        }

        //-----------Write configure Wifi Station------------------------------------
        function writeEEPROMst(){
          if(Empty(document.getElementById("ssidST"), "Please enter ssid!")&&Empty(document.getElementById("passST"), "Please enter password")){
            var ssidST = document.getElementById("ssidST").value;
            var passST = document.getElementById("passST").value;
            var cdhdwifiST = document.getElementsByName("cdhdwifiST");
            for (var i = 0, length = cdhdwifiST.length; i < length; i++) {
                if (cdhdwifiST[i].checked) {
                    cdhdwifiST=cdhdwifiST[i].value;    
                    break;
                }
            }
            xhttp.open("GET","/writeEEPROMst?ssidST="+ssidST+"&passST="+passST+"&cdhdwifiST="+cdhdwifiST,true);
            xhttp.onreadystatechange = process;//nhận reponse 
            xhttp.send();
          }
        }

        //-----------Clear EEPROM------------------------------------
        function clearEEPROM(){
          if(confirm("Do you want to delete all saved wifi configurations?")){
            xhttp.open("GET","/clearEEPROM",true);
            xhttp.onreadystatechange = process;//nhận reponse 
            xhttp.send();
          }
        }

        //-----------Restart ESP8266------------------------------------
        function restartESP(){
          if(confirm("Do you want to reboot the device?")){
            xhttp.open("GET","/restartESP",true);
            xhttp.send();
            alert("Device is restarting! If no wifi is found please press reset!");
          }
        }

        //-----------Write configure sim800a------------------------------------
        function writeEEPROMsim800a(){
          if(Empty(document.getElementById("sdtSend"), "Please enter mobile number!")&&Empty(document.getElementById("smsSend"), "Please enter sms content")&&Empty(document.getElementById("minValueAlert"), "Please enter Min Value!")&&Empty(document.getElementById("maxValueAlert"), "Please enter Max Value!")){
            var sdtgui = document.getElementById("sdtSend").value;
            var smsgui = document.getElementById("smsSend").value;
            var minValueAlert = document.getElementById("minValueAlert").value;
            var maxValueAlert = document.getElementById("maxValueAlert").value;
            var cdhd_alert = document.getElementsByName("cdhdAlert");
            var cdhd_sim800a = document.getElementsByName("cdhdSim800a");
            for (var i = 0, length = cdhd_alert.length; i < length; i++) {
                if (cdhd_alert[i].checked) {
                    cdhd_alert=cdhd_alert[i].value;    
                    break;
                }
            }
            for (var i = 0, length = cdhd_sim800a.length; i < length; i++) {
                if (cdhd_sim800a[i].checked) {
                    cdhd_sim800a=cdhd_sim800a[i].value;    
                    break;
                }
            }
            xhttp.open("GET","/writeEEPROMsim800a?sdtSend="+sdtgui+"&smsSend="+smsgui+ "&minValueAlert="+minValueAlert+"&maxValueAlert="+maxValueAlert+"&cdhdAlert="+cdhd_alert+ "&cdhdSim800a="+cdhd_sim800a,true);
            xhttp.onreadystatechange = process;//nhận reponse 
            xhttp.send();
          }
        }

        //-----------Write configure Led------------------------------------
        function writeEEPROMled(){
          if(Empty(document.getElementById("lowValueLed"), "Please enter Low Value!")&&Empty(document.getElementById("highValueLed"), "Please enter High Value!")){
            var lowValueLed = document.getElementById("lowValueLed").value;
            var highValueLed = document.getElementById("highValueLed").value;
            xhttp.open("GET","/writeEEPROMled?lowValueLed="+lowValueLed+"&highValueLed="+highValueLed,true);
            xhttp.onreadystatechange = process;//nhận reponse 
            xhttp.send();
          }
        }

        //-----------Write configure Relay------------------------------------
        function writeEEPROMrelay(){
          if(Empty(document.getElementById("lowValueRelay"), "Please enter Low Value!")&&Empty(document.getElementById("highValueRelay"), "Please enter High Value")){
            var lowValueRelay = document.getElementById("lowValueRelay").value;
            var highValueRelay = document.getElementById("highValueRelay").value;
            var cdhdRelay = document.getElementsByName("cdhdRelay");
            for (var i = 0, length = cdhdRelay.length; i < length; i++) {
                if (cdhdRelay[i].checked) {
                    cdhdRelay=cdhdRelay[i].value;    
                    break;
                }
            }
            xhttp.open("GET","/writeEEPROMrelay?lowValueRelay="+lowValueRelay+"&highValueRelay="+highValueRelay+"&cdhdRelay="+cdhdRelay,true);
            xhttp.onreadystatechange = process;//nhận reponse 
            xhttp.send();
          }
        }
        
        //-----------Check response -------------------------------------------
        function process(){
          if(xhttp.readyState == 4 && xhttp.status == 200){
            //------Updat data sử dụng javascript----------
            ketqua = xhttp.responseText; 
            alert(ketqua);       
          }
        }
        
       //----------------------------CHECK EMPTY--------------------------------
       function Empty(element, AlertMessage){
          if(element.value.trim()== ""){
            alert(AlertMessage);
            element.focus();
            return false;
          }else{
            return true;
          }
       }
       
       //=====================WEBSOCKET CLIENT===================================
       var Socket;      //Init var Socket
       var ssidAP, passAP, cdhdWiFiAP, ssidST, passST, cdhdWiFiST, sdtSend, smsSend, minValueAlert, maxValueAlert, cdhdAlert, cdhdSim800a, lowValueLed, highValueLed, lowValueRelay, highValueRelay, cdhdRelay;
       function init(){
         //Init websocket
         Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
         //Recieve broadcase from server
         Socket.onmessage = function(event)
         {   
           JSONobj = JSON.parse(event.data);   //Tách dữ liệu json
           ssidAP = JSONobj.ssidAP;
           passAP = JSONobj.passAP;
           cdhdWiFiAP = JSONobj.cdhdWiFiAP;
           ssidST = JSONobj.ssidST;
           passST = JSONobj.passST;
           cdhdWiFiST = JSONobj.cdhdWiFiST;
           sdtSend = JSONobj.sdtSend;
           smsSend = JSONobj.smsSend;
           minValueAlert = JSONobj.minValueAlert;
           maxValueAlert = JSONobj.maxValueAlert;
           cdhdAlert = JSONobj.cdhdAlert;
           cdhdSim800a = JSONobj.cdhdSim800a;
           lowValueLed = JSONobj.lowValueLed;
           highValueLed = JSONobj.highValueLed;
           lowValueRelay = JSONobj.lowValueRelay;
           highValueRelay = JSONobj.highValueRelay;
           cdhdRelay = JSONobj.cdhdRelay;
         }
       }
       
        //----------Gioi han ky tu nhap vao maxlength sms----------
        var smsinput = document.getElementById("smsSend");
        var length = smsinput.getAttribute("maxlength");
        var numberinput = document.getElementById('count');
        numberinput.innerHTML = length;
        smsinput.onkeyup = function () {
          document.getElementById('count').innerHTML = (length - this.value.length);
        };
        
        //----------Test Moduel 800A----------
        function test_sim800a() {
          var cdhd_sim800a = document.getElementsByName("cdhdSim800a");
          for (var i = 0, length = cdhd_sim800a.length; i < length; i++) {
              if (cdhd_sim800a[i].checked) {
                  cdhd_sim800a=cdhd_sim800a[i].value;    
                  break;}
          }
          if(cdhd_sim800a=="0")
          {
            Socket.send("testcall");
            xhttp.open("GET","/testcall",true);
            xhttp.onreadystatechange = process;//nhận reponse 
            xhttp.send();
          }
          else if(cdhd_sim800a=="1")
          {
            Socket.send("testsms");
            xhttp.open("GET","/testsms",true);
            xhttp.onreadystatechange = process;//nhận reponse 
            xhttp.send();
          }
        }

        //----------Test Led----------
        function test_led() {
          Socket.send("testled");
          xhttp.open("GET","/testled",true);
          xhttp.onreadystatechange = process;//nhận reponse 
          xhttp.send();}

        //----------Test Relay----------
        function test_relay() {
          Socket.send("testrelay");
          xhttp.open("GET","/testrelay",true);
          xhttp.onreadystatechange = process;//nhận reponse 
          xhttp.send();}
        
      </script>
   </body> 
  </html>
)=====";




// Setup
void setup() {
  digitalWrite(greenLed,LOW);
  digitalWrite(blueLed,LOW);
  digitalWrite(redLed,LOW);
  Serial.begin(9600);
  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);
  client.setInsecure();
  
  EEPROM.begin(512);       //Khởi tạo bộ nhớ EEPROM
  delay(10);

  dht.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x78>>1);
  display.display();
  delay(500);
  display.clearDisplay();

  pinMode(flashButton,INPUT_PULLUP);
  pinMode(D8,INPUT);
  pinMode(D0,OUTPUT);
  pinMode(greenLed,OUTPUT);
  pinMode(blueLed,OUTPUT);
  pinMode(redLed,OUTPUT);

  read_EEPROM();
  Serial.println("=======================================");
  if(cdhdST=="1"){
    display.setTextColor(WHITE);
    display.setCursor(4,10);
    display.setTextSize(2);
    display.println("Wifi Station mode: enable");
    Serial.println("Wifi Station mode: enable");
    display.display();
    delay(1500);
    display.clearDisplay();
    display.setCursor(4,15);
    display.setTextSize(2);
    display.println("Connecting to WiFi..");
    Serial.println("Connecting to WiFi..");
    display.display();display.clearDisplay();
    delay(1500);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssidST, passST);
    while(WiFi.status() != WL_CONNECTED)
    {
      if (millis()>13000)
      {Serial.println("Connect Wifi failed");
        display.setCursor(4,20);
        display.setTextSize(2);
        display.println("Connect Wifi failed");
        display.display();delay(1000);display.clearDisplay();break;}
      delay(200);
    }}
    if (WiFi.status() == WL_CONNECTED) 
      {Serial.println("Wifi connected");
        display.setCursor(4,20);
        display.setTextSize(2);
        display.println("Wifi connected");
        display.display();delay(1500);
        display.clearDisplay();}
  startWebServer();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  setupSIM800A();
}

// Loop
void loop() {
  if (digitalRead(flashButton)==LOW){
    count+=1;
    if (count > 2){
      WiFi.disconnect();
      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(ipAP, gatewayAP, subnetAP);
      //if(cdhdAP == "1"){WiFi.softAP(ssidAP,passAP,1,false);}else{WiFi.softAP(ssidAP,passAP,1,true);}
      WiFi.softAP(ssidAP,passAP,1,false);
      Serial.println("Configuration mode started");
      modeState=0;}
  }else{count=0;}
  webServer.handleClient();
  webSocket.loop();
  if(Serial.available()){
    while(Serial.available()){
      char inChar = (char)Serial.read();
      bufferSIM800 += inChar;
      if(bufferSIM800.length()>=160){bufferSIM800="";}
    }
    Serial.println(bufferSIM800);
    //Khi có tin nhắn đến thì kiểm tra tin nhắn
    IndexRxdata = bufferSIM800.indexOf("OFF CB");
    if(IndexRxdata >= 0){
      IndexRxdata = -1;
      bufferSIM800="";
      Serial.println("Đã nhận tin nhắn tắt cảnh báo!");
      Serial.println("Đã tắt cảnh báo!");
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("Received SMS: OFF CB");
      display.println("CONFIGURE SIM800A");
      display.println("Operation Mode: Disable");
      display.display();
      display.clearDisplay();
      delay(3000);
      cdhdAlert="1";
      EEPROM.write(342, cdhdAlert[0]);
      EEPROM.commit();
    }
    IndexRxdata = bufferSIM800.indexOf("ON CB0");
    if(IndexRxdata >= 0){
      IndexRxdata = -1;
      bufferSIM800="";
      Serial.println("Đã nhận tin nhắn bật cảnh báo!");
      Serial.println("Đã kích hoạt báo động chế độ Call mobile!");
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("Received SMS: ON CB0");
      display.println("CONFIGURE SIM800A");
      display.println("Operation Mode: Enable Call Mobile");
      display.display();
      display.clearDisplay();
      delay(3000);
      cdhdAlert="0";
      cdhdSim800="0";
      EEPROM.write(342, cdhdAlert[0]);
      EEPROM.write(343, cdhdSim800[0]);
      EEPROM.commit();
    }
    IndexRxdata = bufferSIM800.indexOf("ON CB1");
    if(IndexRxdata >= 0){
      IndexRxdata = -1;
      bufferSIM800="";
      Serial.println("Đã nhận tin nhắn bật cảnh báo!");
      Serial.println("Đã kích hoạt báo động chế độ Send SMS!");
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("Received SMS: ON CB1");
      display.println("CONFIGURE SIM800A");
      display.println("Operation Mode: Enable Send SMS");
      display.display();
      display.clearDisplay();
      delay(3000);
      cdhdAlert="0";
      cdhdSim800="1";
      EEPROM.write(342, cdhdAlert[0]);
      EEPROM.write(343, cdhdSim800[0]);
      EEPROM.commit();
    }
    IndexRxdata = bufferSIM800.indexOf("GET VALUE");
    if(IndexRxdata >= 0){
      IndexRxdata = -1;
      bufferSIM800="";
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("Received SMS: GET VALUE");
      display.display();
      display.clearDisplay();
      delay(1000);
      String s = "Current temperature: " + String(t);
      SIM800SMS(sdtSend, s);
    }
    IndexRxdata = bufferSIM800.indexOf("GET SETTUP");
    if(IndexRxdata >= 0){
      IndexRxdata = -1;
      bufferSIM800="";
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("Received SMS: GET SETUP");
      display.display();
      display.clearDisplay();
      delay(1000);
      String s = "LED CONFIGURATION: Low value: "+String(lowValueLed)+" High value: "+String(highValueLed)+"   RELAY CONFIGURATION: Low value: "+String(lowValueRelay)+" High value: "+String(highValueRelay)+" Mode: ";
      if(cdhdRL=="1"){s += "Auto";}
      else {s += "Manual";}
      s = s + "   SIM800A CONFIGURATION: Min value: "+String(minValueAlert)+" Max value: "+String(maxValueAlert)+" Mode: ";
      if(cdhdAlert=="1"){s += "Disable";}
      else {s += "Enable";}
      s += " Method: ";
      if(cdhdSim800=="1"){s += "Send SMS";}
      else {s += "Call moble";}
      SIM800SMS(sdtSend, s);
    }
    IndexRxdata = bufferSIM800.indexOf("RELAY AUTO");
    if(IndexRxdata >= 0){
      IndexRxdata = -1;
      bufferSIM800="";
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("Received SMS: RELAY AUTO");
      display.println("CONFIGURE RELAY");
      display.println("Operation Mode: Auto");
      display.display();
      display.clearDisplay();
      delay(3000);
      cdhdRL="1";
      EEPROM.write(364, cdhdRL[0]);
      EEPROM.commit();
    }
    IndexRxdata = bufferSIM800.indexOf("RELAY MAN");
    if(IndexRxdata >= 0){
      IndexRxdata = -1;
      bufferSIM800="";
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("Received SMS: RELAY MAN");
      display.println("CONFIGURE RELAY");
      display.println("Operation Mode: Manual");
      display.display();
      display.clearDisplay();
      delay(3000);
      cdhdRL="0";
      EEPROM.write(364, cdhdRL[0]);
      EEPROM.commit();
    }
    IndexRxdata = bufferSIM800.indexOf("RELAY ON");
    if(IndexRxdata >= 0){
      IndexRxdata = -1;
      bufferSIM800="";
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("Received SMS: RELAY ON");
      display.println("CONFIGURE RELAY");
      display.println("Operation Mode: Manual");
      display.println("State: On");
      display.display();
      display.clearDisplay();
      delay(3000);
      cdhdRL="0";
      EEPROM.write(364, cdhdRL[0]);
      EEPROM.commit();
      digitalWrite(relay,HIGH);
    }
    IndexRxdata = bufferSIM800.indexOf("RELAY OFF");
    if(IndexRxdata >= 0){
      IndexRxdata = -1;
      bufferSIM800="";
      display.setCursor(0,0);
      display.setTextSize(1);
      display.println("Received SMS: RELAY OFF");
      display.println("CONFIGURE RELAY");
      display.println("Operation Mode: Manual");
      display.println("State: Off");
      display.display();
      display.clearDisplay();
      delay(3000);
      cdhdRL="0";
      EEPROM.write(364, cdhdRL[0]);
      EEPROM.commit();
      digitalWrite(relay,LOW);
    }
  }
  if (modeState==0)
  {
    display.setCursor(4,20);
    display.setTextSize(2);
    display.println("Configuration mode");
    display.display();
    display.clearDisplay();
    digitalWrite(greenLed,HIGH);
    digitalWrite(blueLed,HIGH);
    digitalWrite(redLed,HIGH);
  }

  if (modeState==1)
  {
    if(cdhdST=="1"){
      if (millis()-timeReConWF>60000){
        timeReConWF=millis();
        if (WiFi.status() != WL_CONNECTED) 
        {Serial.println("Connect Wifi failed");
          display.setCursor(4,20);
          display.setTextSize(2);
          display.println("Connect Wifi failed");
          display.display();delay(1000);
          display.clearDisplay();}
    }}
    if (digitalRead(button)==HIGH) //kiểm tra giá trị từ nút nhấn để điều khiển thiết bị
    {
      if(cdhdRL=="1"){
        Serial.println("Mode Relay change:Auto ==> Manual");
        cdhdRL="0";
        EEPROM.write(364, cdhdRL[0]);
        EEPROM.commit();
        String s ="Mode Relay change:Auto ==> Manual";
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println(s);
        display.display();delay(2000);
        display.clearDisplay();
        SIM800SMS(sdtSend, s);
      }
      if (butPress==false)
      {if (digitalRead(relay)==HIGH) {digitalWrite(relay,LOW);Serial.println("Relay: OFF");}
        else {digitalWrite(relay,HIGH);Serial.println("Relay: ON");}}
      butPress=true;
      delay(50);}
    if (digitalRead(button)==LOW)
    {butPress = false;
      delay(50);}
    if (millis()-timeDht>5000){
    h = dht.readHumidity();
    t = dht.readTemperature();   // Read temperature as Celsius
    if (isnan(h) || isnan(t)) {
      h=-1.0;
      t=-1.0;
    }
    Serial.print("Temperature: ");Serial.print(t);Serial.println(" *C         ");//Serial.print("Humidity: ");Serial.print(h);Serial.println(" %");
    showTemp(t,h);
    timeDht=millis();
    if (t<lowValueLed.toFloat()){
      digitalWrite(redLed,LOW);
      digitalWrite(blueLed,LOW);
      digitalWrite(greenLed,HIGH);
    }
    else if (t>highValueLed.toFloat()){
      digitalWrite(blueLed,LOW);
      digitalWrite(greenLed,LOW);
      digitalWrite(redLed,HIGH);
    }
    else{
      digitalWrite(blueLed,HIGH);
      digitalWrite(greenLed,LOW);
      digitalWrite(redLed,LOW);}}
      if(millis() - timeGGS >= 20000) //cách 1 khoảng thời gian thì gửi dữ liệu lên google sheet
      {
        timeGGS = millis();
        if(cdhdST=="1"&&WiFi.status() == WL_CONNECTED)sendDataGGS(String(t), String(h)); }
      if(cdhdRL=="1"){
        if (t>lowValueRelay.toFloat()&&t<highValueRelay.toFloat()&&t!=-1.0){digitalWrite(relay,HIGH);}
        else {digitalWrite(relay,LOW);}
      }
      if(cdhdAlert=="0"){
        if (millis() - timeReAlert >= 60000)
        {
          if (t<minValueAlert.toFloat()||t>maxValueAlert.toFloat()&&t!=-1.0)
          {
            if (cdhdSim800=="0")
            {
              SIM800Call(sdtSend);
            }
            else
            {
              SIM800SMS(sdtSend, smsSend);
            }
          }
          timeReAlert = millis();
        }
      }
   }
}

//==========CHƯƠNG TRÌNH CON===================================//
//------------Đọc bộ nhớ EEPROM--------------------
void read_EEPROM(){
  Serial.println("Reading EEPROM...");

  Serial.println("==================Wifi Access Point Configuration=====================");
  if(EEPROM.read(0)!=0){
    ssidAP = "";
    passAP = "";
    cdhdAP = "";
    for (int i=0; i<32; ++i){
      ssidAP += char(EEPROM.read(i));
    }
    Serial.print("SSID AP: ");
    Serial.println(ssidAP);
    for (int i=32; i<96; ++i){
      passAP += char(EEPROM.read(i));
    }
    Serial.print("PASSWORD: ");
    Serial.println(passAP);
    cdhdAP = char(EEPROM.read(96));
    Serial.print("Mode: ");
    if(cdhdAP=="0"){
      Serial.println("Hide");
    }else if(cdhdAP=="1"){
      Serial.println("Show");
    }
    ssidAP = ssidAP.c_str();
    passAP = passAP.c_str();
    }else{
    Serial.println("Data wifi AP mode not found!");}


  Serial.println("==================Wifi Station Configuration=====================");
  if(EEPROM.read(97)!=0){
    ssidST = "";
    passST = "";
    cdhdST = "";
    for (int i=97; i<129; ++i){
      ssidST += char(EEPROM.read(i));
    }
    Serial.print("SSID Station: ");
    Serial.println(ssidST);
    for (int i=129; i<161; ++i){
      passST += char(EEPROM.read(i));
    }
    Serial.print("PASSWORD: ");
    Serial.println(passST);
    cdhdST = char(EEPROM.read(161));
    Serial.print("Mode: ");
    if(cdhdST=="1"){
      Serial.println("Enable");
    }else if(cdhdST=="0"){
      Serial.println("Disable");
    }
    ssidST = ssidST.c_str();
    passST = passST.c_str();
  }else{
    Serial.println("Data wifi station mode not found!");}

  
  Serial.println("==============SIM800A Configuration===============");
  if(EEPROM.read(163)!=0){
    sdtSend="";
    smsSend="";
    minValueAlert="";
    maxValueAlert="";
    cdhdAlert="";
    cdhdSim800="";
    for (int i=162; i<172; ++i){
      sdtSend += char(EEPROM.read(i));
    }
    Serial.print("Phone number: ");
    Serial.println(sdtSend);
    for (int i=172; i<332; ++i){
      smsSend += char(EEPROM.read(i));
    }
    Serial.print("SMS Content: ");
    Serial.println(smsSend);
    for (int i=332; i<337; ++i){
      minValueAlert += char(EEPROM.read(i));
    }
    Serial.print("Min Value: ");
    Serial.println(minValueAlert);
    for (int i=337; i<342; ++i){
      maxValueAlert += char(EEPROM.read(i));
    }
    Serial.print("Max Value: ");
    Serial.println(maxValueAlert);
    cdhdAlert = char(EEPROM.read(342));
    Serial.print("Method: ");
    if(cdhdAlert=="0"){
      Serial.println("Enable");
    }else if(cdhdAlert=="1"){
      Serial.println("Disable");
    }
    cdhdSim800 = char(EEPROM.read(343));
    Serial.print("Method: ");
    if(cdhdSim800=="0"){
      Serial.println("Call Mobile");
    }else if(cdhdSim800=="1"){
      Serial.println("Send SMS");
    }
    sdtSend = sdtSend.c_str();
    smsSend = smsSend.c_str();
    minValueAlert = minValueAlert.c_str();
    maxValueAlert = maxValueAlert.c_str();
  }else{
    Serial.println("Data sim800a not found!");}


  Serial.println("==============Led Configuration===============");
  if(EEPROM.read(344)!=0){
    lowValueLed="";
    highValueLed="";
    for (int i=344; i<349; ++i){
      lowValueLed += char(EEPROM.read(i));
    }
    Serial.print("Low Value: ");
    Serial.println(lowValueLed);
    for (int i=349; i<354; ++i){
      highValueLed += char(EEPROM.read(i));
    }
    Serial.print("High Value: ");
    Serial.println(highValueLed);
    lowValueLed = lowValueLed.c_str();
    highValueLed = highValueLed.c_str();
  }else{
    Serial.println("Data led not found!");}

    
  Serial.println("==============Relay Configuration===============");
  if(EEPROM.read(354)!=0){
    lowValueRelay="";
    highValueRelay="";
    cdhdRL="";
    for (int i=354; i<359; ++i){
      lowValueRelay += char(EEPROM.read(i));
    }
    Serial.print("Low Value: ");
    Serial.println(lowValueRelay);
    for (int i=359; i<364; ++i){
      highValueRelay += char(EEPROM.read(i));
    }
    Serial.print("High Value: ");
    Serial.println(highValueRelay);
    cdhdRL = char(EEPROM.read(364));
    Serial.print("Mode: ");
    if(cdhdRL=="0"){
      Serial.println("Manual");
    }else if(cdhdRL=="1"){
      Serial.println("Auto");
    }
    lowValueRelay = lowValueRelay.c_str();
    highValueRelay = highValueRelay.c_str();
    cdhdRL = cdhdRL.c_str();
  }else{
    Serial.println("Data relay not found!");}}


//----------------WEB SERVER-----------------------
void startWebServer(){
  webServer.on("/",[]{
    String s = FPSTR(MainPage);
    webServer.send(200,"text/html",s);
  });

  
  webServer.on("/writeEEPROMap",[]{
    ssidAP = webServer.arg("ssidAP");
    passAP = webServer.arg("passAP");
    cdhdAP = webServer.arg("cdhdwifiAP");
    for (int i = 0; i < 97; ++i) {
      EEPROM.write(i, 0);           
      delay(10);
    }
    for (int i = 0; i < ssidAP.length(); ++i) {
      EEPROM.write(i, ssidAP[i]);
    }
    for (int i = 0; i < passAP.length(); ++i) {
      EEPROM.write(32 + i, passAP[i]);
    }
    EEPROM.write(96, cdhdAP[0]);
    EEPROM.commit();
    Serial.println("Writed to EEPROM!");
    Serial.print("SSID AP: ");
    Serial.println(ssidAP);
    Serial.print("PASS: ");
    Serial.println(passAP);
    Serial.print("Mode: ");
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("CONFIGURE WIFI ACCESS POINT");
    display.print("SSID AP: ");
    display.println(ssidAP);
    display.print("PASS: ");
    display.println(passAP);
    display.print("Mode: ");
    if(cdhdAP=="0"){
      Serial.println("Hide");
      display.println("Hide");
    }else if(cdhdAP=="1"){
      Serial.println("Show");
      display.println("Show");
    }
    display.display();
    display.clearDisplay();
    delay(5000);
    String s = "Wifi Access Point configuration saved!";
    webServer.send(200, "text/html", s);
  });


  webServer.on("/writeEEPROMst",[]{
    ssidST = webServer.arg("ssidST");
    passST = webServer.arg("passST");
    cdhdST = webServer.arg("cdhdwifiST");
    for (int i = 97; i < 162; ++i) {
      EEPROM.write(i, 0);           
      delay(10);
    }
    for (int i = 0; i < ssidST.length(); ++i) {
      EEPROM.write(97+i, ssidST[i]);
    }
    for (int i = 0; i < passST.length(); ++i) {
      EEPROM.write(129+i, passST[i]);
    }
    EEPROM.write(161, cdhdAP[0]);
    EEPROM.commit();
    Serial.println("Writed to EEPROM!");
    Serial.print("SSID Station: ");
    Serial.println(ssidST);
    Serial.print("PASS: ");
    Serial.println(passST);
    Serial.print("Mode: ");
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("CONFIGURE WIFI STATION");
    display.print("SSID Station: ");
    display.println(ssidST);
    display.print("PASS: ");
    display.println(passST);
    display.print("Mode: ");
    if(cdhdST=="0"){
      Serial.println("Disable");
      display.println("Disable");
    }else if(cdhdST=="1"){
      Serial.println("Enable");
      display.println("Enable");
    }
    display.display();
    display.clearDisplay();
    delay(3000);
    String s = "Wifi Station configuration saved!";
    webServer.send(200, "text/html", s);
    
  });

  
  webServer.on("/writeEEPROMsim800a",[]{
    sdtSend = webServer.arg("sdtSend");
    smsSend = webServer.arg("smsSend");
    minValueAlert = webServer.arg("minValueAlert");
    maxValueAlert = webServer.arg("maxValueAlert");
    cdhdAlert = webServer.arg("cdhdAlert");
    cdhdSim800 = webServer.arg("cdhdSim800a");
    Serial.println("Clear EEPROM!");
    for (int i = 162; i < 344; ++i) {
      EEPROM.write(i, 0);           
      delay(10);
    }
    for (int i = 0; i < sdtSend.length(); ++i) {
      EEPROM.write(162 + i, sdtSend[i]);
    }
    for (int i = 0; i < smsSend.length(); ++i) {
      EEPROM.write(172+i, smsSend[i]);
    }
    for (int i = 0; i < minValueAlert.length(); ++i) {
      EEPROM.write(332+i, minValueAlert[i]);
    }
    for (int i = 0; i < maxValueAlert.length(); ++i) {
      EEPROM.write(337+i, maxValueAlert[i]);
    }
    EEPROM.write(342, cdhdAlert[0]);
    EEPROM.write(343, cdhdSim800[0]);
    EEPROM.commit();
    Serial.println("Writed to EEPROM!");
    Serial.print("Số điện thoại: ");
    Serial.println(sdtSend);
    Serial.print("SMS: ");
    Serial.println(smsSend);
    Serial.print("Mode: ");
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("CONFIGURE SIM800A");
    display.print("Phone number: ");
    display.println(sdtSend);
    display.print("Message content: ");
    display.println(smsSend);
    display.print("Mode: ");
    if(cdhdAlert=="0"){
      Serial.print("Enable      ");
      display.print("Enable ");
    }else if(cdhdAlert=="1"){
      Serial.print("Disable     ");
      display.print("Disable ");}
    if(cdhdSim800=="0"){
      Serial.println("Call Mobile");
      display.println("Call");
    }else if(cdhdSim800=="1"){
      Serial.println("Send SMS");
      display.println("SMS");}
    display.display();
    display.clearDisplay();
    delay(3000);
    String s = "Sim800A configuration saved!";
    webServer.send(200, "text/html", s);
  });

  webServer.on("/writeEEPROMled",[]{
    lowValueLed = webServer.arg("lowValueLed");
    highValueLed = webServer.arg("highValueLed");
    for (int i = 344; i < 354; ++i) {
      EEPROM.write(i, 0);           
      delay(10);
    }
    for (int i = 0; i < lowValueLed.length(); ++i) {
      EEPROM.write(344+i, lowValueLed[i]);
    }
    for (int i = 0; i < highValueLed.length(); ++i) {
      EEPROM.write(349 + i, highValueLed[i]);
    }
    EEPROM.commit();
    Serial.println("Writed to EEPROM!");
    Serial.print("Low Value: ");
    Serial.println(lowValueLed);
    Serial.print("High Value: ");
    Serial.println(highValueLed);
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("CONFIGURE LED");
    display.print("Low Value: ");
    display.println(lowValueLed);
    display.print("High Value: ");
    display.println(highValueLed);
    display.display();
    display.clearDisplay();
    delay(3000);
    String s = "Led configuration saved!";
    webServer.send(200, "text/html", s);
  });


  webServer.on("/writeEEPROMrelay",[]{
    lowValueRelay = webServer.arg("lowValueRelay");
    highValueRelay = webServer.arg("highValueRelay");
    cdhdRL = webServer.arg("cdhdRelay");
    for (int i = 354; i < 365; ++i) {
      EEPROM.write(i, 0);           
      delay(10);
    }
    for (int i = 0; i < lowValueRelay.length(); ++i) {
      EEPROM.write(i+354, lowValueRelay[i]);
    }
    for (int i = 0; i < highValueRelay.length(); ++i) {
      EEPROM.write(359 + i, highValueRelay[i]);
    }
    EEPROM.write(364, cdhdRL[0]);
    EEPROM.commit();
    Serial.println("Writed to EEPROM!");
    Serial.print("Low Value: ");
    Serial.println(lowValueRelay);
    Serial.print("High Value: ");
    Serial.println(highValueRelay);
    Serial.print("Mode: ");
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("CONFIGURE RELAY");
    display.print("Low Value: ");
    display.println(lowValueRelay);
    display.print("High Value: ");
    display.println(highValueRelay);
    display.print("Mode: ");
    if(cdhdRL=="0"){
      Serial.println("Manual");
      display.println("Manual");
    }else if(cdhdRL=="1"){
      Serial.println("Auto");
      display.println("Auto");
    }
    display.display();
    display.clearDisplay();
    delay(3000);
    String s = "Relay configuration saved!";
    webServer.send(200, "text/html", s);
  });

  
  webServer.on("/restartESP",[]{
    ESP.restart();
  });

  
  webServer.on("/clearEEPROM",[]{
    for (int i = 0; i < 512; ++i) {
      EEPROM.write(i, 0);           
      delay(10);
    }
    EEPROM.commit();
    display.setCursor(0,0);
    display.setTextSize(2);
    display.println("Cleared EEPROM");
    display.display();
    display.clearDisplay();
    delay(3000);
    String s = "Device has been reset!";
    webServer.send(200,"text/html", s);
    Serial.println("Clear EEPROM!");
  });


  webServer.on("/testled",[]{    
  String s = "Led tested!";
  webServer.send(200, "text/html", s);
  Serial.println(s);
  });


  webServer.on("/testrelay",[]{    
  String s = "Relay tested!";
  webServer.send(200, "text/html", s);
  Serial.println(s);
  });


  webServer.on("/testcall",[]{    
  String s = "Call tested!";
  webServer.send(200, "text/html", s);
  Serial.println(s);
  });


  webServer.on("/testsms",[]{    
  String s = "SMS tested!";
  webServer.send(200, "text/html", s);
  Serial.println(s);
  });

  
  webServer.begin();
  Serial.println("Web Server is started!");
}

//---------------------WEBSOCKET---------------------------------/
void webSocketEvent(uint8_t num, WStype_t type,uint8_t * payload,size_t length){
  String payloadString = (const char *)payload;
  //Serial.print("payloadString= ");Serial.println(payloadString);
  if(payloadString == "testcall"){
    SIM800Call(sdtSend);
  }
  if(payloadString == "testsms"){
    SIM800SMS(sdtSend, smsSend);
  }
  if(payloadString == "testled"){
    testLed();
  }
  if(payloadString == "testrelay"){
    testRelay();
  }
}

void testLed(){
  digitalWrite(D5,LOW);digitalWrite(D6,LOW);digitalWrite(D7,LOW);
  display.setCursor(4,20);
  display.setTextSize(2);
  display.println("Test Led");
  display.display();
  display.clearDisplay();delay(1000);
  digitalWrite(redLed,HIGH);digitalWrite(blueLed,LOW);digitalWrite(greenLed,LOW);delay(2000);
  digitalWrite(redLed,LOW);digitalWrite(blueLed,HIGH);digitalWrite(greenLed,LOW);delay(2000);
  digitalWrite(redLed,LOW);digitalWrite(blueLed,LOW);digitalWrite(greenLed,HIGH);delay(2000);
  digitalWrite(redLed,HIGH);digitalWrite(blueLed,HIGH);digitalWrite(greenLed,LOW);delay(2000);
  digitalWrite(redLed,HIGH);digitalWrite(blueLed,LOW);digitalWrite(greenLed,HIGH);delay(2000);
  digitalWrite(redLed,LOW);digitalWrite(blueLed,HIGH);digitalWrite(greenLed,HIGH);delay(2000);
  digitalWrite(redLed,HIGH);digitalWrite(blueLed,HIGH);digitalWrite(greenLed,HIGH);delay(2000);
  display.setCursor(4,20);
  display.println("End Test Led");
  display.display();
  display.clearDisplay();
  delay(500);
}

void testRelay(){
  display.setCursor(4,20);
  display.setTextSize(2);
  display.println("Test Relay");
  display.display();
  display.clearDisplay();delay(1000);
  digitalWrite(relay,HIGH);delay(1000);
  digitalWrite(relay,LOW);delay(1000);
  digitalWrite(relay,HIGH);delay(1000);
  digitalWrite(relay,LOW);delay(1000);
  display.setCursor(4,20);
  display.println("End Test Relay");
  display.display();
  display.clearDisplay();
  delay(500);
}

void setupSIM800A(){
  Serial.println("ATE0"); //Tắt chế độ echo khi gửi lệnh đi
  delay(1000);
  Serial.println("AT+IPR=9600"); //Cài tốc độ baud 9600
  delay(1000);
  Serial.println("AT+CMGF=1"); //Hiển thị tin nhắn ở chế độ txt
  delay(1000);
  Serial.println("AT+CLIP=1"); //Hiển thị số điện thoại gọi đến
  delay(1000);
  Serial.println("AT+CNMI=2,2"); //Hiển thị trực tiếp nội dung tin nhắn gửi đến
  delay(1000);
}
void SIM800Call(String sdt){
  Serial.println("ATD" + sdt + ";");
  display.setCursor(0,0);
  display.setTextSize(2);
  display.print("Calling to");
  display.println(sdt);
  display.println(".....");
  display.display();
  display.clearDisplay();
  delay(1000);
  Serial.print("Đang thực hiện cuộc gọi đến số: ");
  Serial.println(sdt);
  delay(19000);
  Serial.println("ATH");
}
void SIM800SMS(String phone, String content){
  Serial.println("AT+CMGS=\"" + phone + "\"");     // Lenh gui tin nhan
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("Sent SMS to: ");
  display.println(phone);
  display.println("Content:");
  display.println(content);
  display.display();
  display.clearDisplay();
  delay(3000);                                     // Cho ky tu '>' phan hoi ve 
  Serial.print(content);                           // Gui noi dung
  Serial.print((char)26);                          // Gui Ctrl+Z hay 26 de ket thuc noi dung tin nhan va gui tin di
  delay(5000);                                     // delay 5s
  Serial.print("Đã gủi tin nhắn đến số: ");
  Serial.println(phone);
  Serial.print("Nội dung tin nhắn: ");
  Serial.println(content);
}

void showTemp(float temp,float hud) {
  //display.drawBitmap(13, 40, humidityImage, 128, 64, WHITE);
  display.drawBitmap(10, 35, tempImage, 128, 64, WHITE);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(4,2);
  display.println("Temperature");
  display.setTextSize(2);
  //display.setCursor(40,45);
  //display.print(hud);
  //display.println("%");
  display.setCursor(35,40);
  display.print(temp);
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");
  display.display();
  display.clearDisplay();
  delay(100);
}

void sendDataGGS(String t, String h) //hàm gửi dữ liệu lên google sheet
{
  //Serial.println("==========");Serial.print("connecting to ");Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + t;
  //Serial.print("requesting URL: ");Serial.println(url); // https://script.google.com/macros/s/AKfycbzi-68VaLiogLsrnHLrhMxBUxrdE4vXLN-ki-lbVzHtvM4S-fTAsv5kTdSzj1xj6XeQ/exec?temperature=23.80
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");
  Serial.println("request to Google Sheet sent");
  display.setCursor(0,0);
  display.setTextSize(2);
  display.println("request to Google Sheet sent");
  display.display();
  display.clearDisplay();
  delay(500);
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;}
  }
  String line = client.readStringUntil('\n');
  //if (line.startsWith("24d")) {Serial.println("esp8266/Arduino CI successfull!");} 
  //else {Serial.println("esp8266/Arduino CI has failed");}
  //Serial.print("reply was : ");Serial.println(line);Serial.println("closing connection");Serial.println("==========");Serial.println();
  client.stop();
}
