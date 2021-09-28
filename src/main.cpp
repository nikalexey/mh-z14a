#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>


#include "mhz.h"
HardwareSerial port(2);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3c  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

MHZ sensor(&port, MHZ14A);
String warming_progress_bar = "*";

void setup() {
  Serial.begin(115200);
  port.begin(9600);
  Serial.println("Start");
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 24);
  display.println(F("Quadcode"));
  display.display();
  sleep(5.5);

  // sensor.setDebug(true);
}


void loop() {
  static auto idx = 0;
  display.clearDisplay();
  if (sensor.isReady()) {
    auto ppm = sensor.readCO2UART();
    display.setCursor(15, 8);
    display.println(F("CO2 PPM: "));
    display.setCursor(32, 32);
    display.println(ppm, 10);
    display.display();
    Serial.print("PPM: ");
    Serial.println(ppm);
    delay(60000);
  } else {
    display.setCursor(0, 8);
    display.println(F("WARMING UP"));
    display.setCursor(0, 38);
    display.println(warming_progress_bar);
    display.display();
    Serial.print("Not Ready ");
    Serial.println(++idx);
    warming_progress_bar += "*";
    delay(19000);
  }
}