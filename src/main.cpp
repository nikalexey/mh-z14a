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
#define DIM_DISPLAY false // you can specify true for little bit less bright display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

MHZ sensor(&port, MHZ14A);
String warming_progress_bar = "*";

#include "codic.cpp"

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
  while (!sensor.isReady()) {
    display.drawRect(0, 54, 128, 10, 1);
    auto x = map(millis(), 0, (3 * 60 - 5) * 1000, 1, 126);
    display.fillRect(1, 55, x, 9, 1);
    display.display();
  }
}


void printCO2(int ppm) {
  display.clearDisplay();
  display.setCursor(15, 8);
  display.println(F("CO2 PPM: "));
  display.setCursor(32, 32);
  display.println(ppm, 10);
  display.display();
  Serial.print("PPM: ");
  Serial.println(ppm);
}

void loop() {
  if (sensor.isReady()) {
    printCO2(sensor.readCO2UART());
    delay(1000);
  }
}