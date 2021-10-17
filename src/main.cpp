#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <time.h>
#include <WebServer.h>

#include "mhz.h"
HardwareSerial port(2);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3c  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define DIM_DISPLAY false // you can specify true for little bit less bright display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
WebServer webServer(80);

MHZ sensor(&port, MHZ14A);
String warming_progress_bar = "*";

const char* ssid     = "tochko";     
const char* password = "password";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3*3600;
const int   daylightOffset_sec = 0;

int ppm,temp;

#include "codic.cpp"

String printJson() {
  char response[100];
  sprintf(response, "{ \"ppm\": \"%d\", \"temperature\": \"%d\" }", ppm, temp);
  Serial.println(response);  
  webServer.send(200, "application/json", response);
  return response;
}

void setup() {
  Serial.begin(115200);
  port.begin(9600);
  Serial.println("Start");
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  display.dim(DIM_DISPLAY);
  display.clearDisplay();
  display.drawBitmap(0, 0, 
      codic_bitmap, codic_width, codic_height, 1);
  display.display();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(SCREEN_WIDTH/2+6, 15);
  display.println(F("Quad"));
  display.setCursor(SCREEN_WIDTH/2+6, 35);
  display.println(F("code"));
  display.display();
  sleep(5);

  sensor.setDebug(true);
  display.clearDisplay();
  display.setCursor(5, 8);
  display.println(F("WARMING UP"));
  WiFi.begin(ssid, password);             
  while (!sensor.isReady()) {
    display.drawRect(0, 54, 128, 10, 1);
    auto x = map(millis(), 0, (3 * 60 - 5) * 1000, 1, 126);
    display.fillRect(1, 55, x, 9, 1);
    display.display();
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connection established");  
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP()); 
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    webServer.begin();
    webServer.on(F("/"), printJson);
  }
  else {
    Serial.println("Connection wasn't established");  
  }
}

void printCO2(int ppm, int temp, char time_str[9]) {
  display.clearDisplay();
  display.setCursor(14, 4);
  display.println("CO2: " + String(ppm));
  display.setCursor(1, 24 );
  display.println("Temp: " + String(temp));
  Serial.print("PPM: ");
  Serial.println(ppm);
  Serial.print("Temp: ");
  Serial.println(temp);  
  if (time_str[0]>0) {
    display.setCursor(14, 50);
    display.println(String(time_str));
    Serial.print("Time: ");
    Serial.println(time_str);  
  }
  display.display();
}

void loop() {
  struct tm timeinfo;
  char time_str[9];
  String json_str;
  time_str[0] = 0;
  if (WiFi.status() == WL_CONNECTED) {
    if (getLocalTime(&timeinfo)) {          
        strftime(time_str, 9, "%H:%M:%S", &timeinfo);
    }
    webServer.handleClient();
  }
  if (sensor.isReady()) {
    ppm = sensor.readCO2UART();
    temp = sensor.getLastTemperature();
  }
  printCO2(ppm,temp,time_str);
  delay(500);
}