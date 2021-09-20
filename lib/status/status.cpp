// #include <Arduino.h>

#include <Adafruit_DotStar.h>

#include "status.h"

Adafruit_DotStar pixel(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);


void trafficOn() {
  digitalWrite(LED, HIGH);
}

void trafficOff() {
  digitalWrite(LED, LOW);
}

void initStatus() {
  pixel.begin();
  pixel.setBrightness(BRIGHTNESS);
  statusStart();
}

void statusStart() {
  pixel.setPixelColor(0, pixel.Color(0, 0, 150));
  pixel.show();
}

void statusOk() {
  pixel.setPixelColor(0, pixel.Color(150, 0, 0));
  pixel.show();
}

void statusError() {
  pixel.setPixelColor(0, pixel.Color(0, 150, 0));
  pixel.show();
}

