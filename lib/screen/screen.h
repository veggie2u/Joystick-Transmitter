#ifndef SCREEN_H 
#define SCREEN_H 

#include <Arduino.h>

#define SCREEN_WIDTH    128 // OLED display width, in pixels
#define SCREEN_HEIGHT   64 // OLED display height, in pixels
#define OLED_RESET      -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS  0x3C 

void initOled();
void doTheOledThing(uint8_t thisNode, uint8_t otherNode, int8_t rssi, unsigned long current_millis, unsigned long previous_millis);

#endif