#include <Arduino.h>

/*
  Joystick transmitter. Sends x/y and buttons using RF9X Lora radio.
  Listens for a ping from reciever.
*/

#include <SPI.h>
#include "main.h"
#if CWW_IS_LORA == true
#include <RH_RF95.h>
const int MAX_MESSAGE_LEN = RH_RF95_MAX_MESSAGE_LEN;
RH_RF95 radio(RADIO_CS, RADIO_INT);
#else
#include <RH_RF69.h>
const int MAX_MESSAGE_LEN = RH_RF69_MAX_MESSAGE_LEN;
RH_RF69 radio(RADIO_CS, RADIO_INT);
#endif
#include <RHReliableDatagram.h>
#include <utils.h>
#include <debug.h>
#include <status.h>
#include <controls.h>
#include <screen.h>

#define NODEID        2    // The unique identifier of this node
#define RECEIVER      1    // The recipient of packets

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(radio, NODEID);

//int const MESSAGE_LEN = MAX_MESSAGE_LEN;
// for ACK message
uint8_t buf[MAX_MESSAGE_LEN];

char ping[] = "ping";
uint8_t ping_ack[] = "ok";
unsigned long current_millis;
unsigned long previous_millis;  // for calculating ping time
char charBuf[100]; // for sprintf

// send the joystick/button data to the reciever/robot
void sendData() {
  debugD("Sending ");
  Packet_Packed packedData = getPackedData();
  debugD(sizeof(packedData));
  debugD(" bytes - ");    
  if (manager.sendtoWait((uint8_t *)(&packedData), sizeof(packedData), RECEIVER)) {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from)) {
      buf[len] = 0; // zero out remaining string
      sprintf(charBuf, "Got reply from #%i [RSSI :%i] : ", from, radio.lastRssi());
      debugD(charBuf); debuglnD((char*)buf);
      statusOk();
      previous_millis = current_millis;      
      current_millis = millis();
    } else {
      debuglnD("No reply, is anyone listening?");
      statusError();
    }
  } else {
    debuglnD("Sending failed (no ack)");
    statusError();
  }
}

void oled() {
  doTheOledThing(NODEID, RECEIVER, (int16_t)radio.lastRssi(), current_millis, previous_millis);
}

// check to see if the reciever/robot is trying to send us a ping
void checkForPing() {
  uint8_t len = sizeof(buf);
  uint8_t from;
  if (manager.recvfromAck(buf, &len, &from)) {
    trafficOn();
    buf[len] = 0;
    debugD("Got msg from node "); debugD(from); debugD(": ");
    for (byte i = 0; i < len; i++) {
      debugD((char)buf[i]);
    }
    debugD(", ");
    if (strcmp((char *)&buf, (char *)&ping) != 0) {
      debuglnD("Not a ping.");
      statusError();
    } else {
      previous_millis = current_millis;      
      current_millis = millis(); 
      debuglnD("ok");
      statusOk();     
    }

    if (!manager.sendtoWait(ping_ack, sizeof(ping_ack), from)) {
      debuglnD("Sending ping response failed (no ack)");
      statusError();
    }
    trafficOff();
    oled();
  } else {
    if ((millis() - previous_millis) > 8000) {
      oled();
    }            
  }
}

/* ************************
 * Setup
 * ************************/
void setup() {
  Serial.begin(SERIAL_BAUD);
  initStatus();
  initOled();
  initControls();
  
  // Initial State
  pinMode(RADIO_RST, OUTPUT);
  
  sprintf(charBuf, "Feather %s Transmitter", CWW_RADIO_TYPE);
  debuglnD(charBuf);

  // Hard Reset the radio module
  #if CWW_LORA_RADIO == true
  digitalWrite(RADIO_RST, HIGH);
  delay(10);
  digitalWrite(RADIO_RST, LOW);
  delay(10);
  digitalWrite(RADIO_RST, HIGH);
  delay(10);
  #else
  digitalWrite(RADIO_RST, LOW);
  delay(10);
  digitalWrite(RADIO_RST, HIGH);
  delay(10);
  digitalWrite(RADIO_RST, LOW);
  delay(10);
  #endif

  if (!manager.init()) {
    sprintf(charBuf, "%s radio init failed", CWW_RADIO_TYPE);
    debuglnD(charBuf);
    statusError();
    while (1);
  }
  sprintf(charBuf, "%s radio init OK!", CWW_RADIO_TYPE);
  debuglnD(charBuf);

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  if (!radio.setFrequency(CWW_RADIO_FREQ)) {
    debuglnD("setFrequency failed");
    statusError();
  }

  radio.setTxPower(CWW_RADIO_POWER, CWW_IS_RFM69HCW);  // range from 14-23 for power
  
  #if CWW_IS_LORA_RADIO == false
  radio.setEncryptionKey(ENCRYPTION_KEY);
  #endif

  delay(2000); // Pause for 2 seconds
  char charBuf[100]; // for sprintf
  sprintf(charBuf, "Transmitting %s radio @%.1f, Mhz", CWW_RADIO_TYPE, CWW_RADIO_FREQ);
  debuglnD(charBuf);

  oled();
}

/* **************************
 * Continuous Loop
 * **************************/
void loop() {
  Packet data = readJoystick();
  setData(data);
  if (hasJoystickChanged(data)) {
    trafficOn();
    printJoystick(data);
    packData();
    sendData();
    copyJoystickData(data);
    trafficOff();
  } else {
    Serial.flush();
    //radio.sleep();
  }
  checkForPing();
}
