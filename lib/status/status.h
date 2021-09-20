// definitions and functions to handle the status led

#ifndef STATUS_H 
#define STATUS_H 

#define NUMPIXELS       1 // onboard pixel
#define DATAPIN         8
#define CLOCKPIN        6
#define BRIGHTNESS      100

#define LED             13  // onboard blinky

#define SCREEN_WIDTH    128 // OLED display width, in pixels
#define SCREEN_HEIGHT   64 // OLED display height, in pixels
#define OLED_RESET      -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS  0x3C 

void trafficOn();
void trafficOff();

void initStatus();
void statusStart();
void statusOk();
void statusError();

#endif
