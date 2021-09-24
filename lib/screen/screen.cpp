#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include <status.h>
#include "screen.h"

Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void initOled() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    statusError();
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
}

// write data to the oled screen
void doTheOledThing(uint8_t thisNode, uint8_t otherNode, int16_t rssi, unsigned long current_millis, unsigned long previous_millis) {
  char charBuf[40];
  float seconds;
  // Adafruit_SSD1306 display = getDisplay();
  display.clearDisplay();
  // line 1
  display.setCursor(0, 0);     // Start at top-left corner  
  display.write("Joystick Transmitter");
  // line 2
  display.setCursor(0, 9);
  sprintf(charBuf, "TX: %i => Rx: %i", thisNode, otherNode);
  display.write(charBuf);
  // line 3
  display.setCursor(0,16);
  int value2 = rssi;
  sprintf(charBuf, "RSSI: %i", value2); 
  display.write(charBuf);
  // line 4
  display.setCursor(0, 24);
  seconds = (current_millis - previous_millis) / 1000.0;
  sprintf(charBuf, "Ping: %.2f (ms)", seconds);
  display.write(charBuf);

  // line 5
  display.setCursor(0, 32);
  seconds = (millis() - current_millis) / 1000.0;
  sprintf(charBuf, "Last: %.2f (ms)", seconds);
  display.write(charBuf);

  display.display();
}
