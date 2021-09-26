#include <Arduino.h>

/*
  Joystick transmitter. Sends x/y and buttons using RF69 radio.
  Listens for a ping from reciever.
*/

#define DEBUGLEVEL 4

#include "main.h"
#include <SPI.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>
#include <utils.h>
#include <debug.h>
#include <status.h>
#include <controls.h>
#include <screen.h>

#define NODEID        2    // The unique identifier of this node
#define RECEIVER      1    // The recipient of packets

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, NODEID);

// for ACK message
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];

char ping[] = "ping";
uint8_t ping_ack[] = "ok";
unsigned long current_millis;
unsigned long previous_millis;  // for calculating ping time

// send the joystick/button data to the reciever/robot
void sendData() {
  char charBuf[100]; // for sprintf
  debugD("Sending ");
  Packet_Packed packedData = getPackedData();
  debugD(sizeof(packedData));
  debugD(" bytes - ");    
  if (rf69_manager.sendtoWait((uint8_t *)(&packedData), sizeof(packedData), RECEIVER)) {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (rf69_manager.recvfromAckTimeout(buf, &len, 2000, &from)) {
      buf[len] = 0; // zero out remaining string
      sprintf(charBuf, "Got reply from #%i [RSSI :%i] : %s", from, rf69.lastRssi(), (char*)buf);
      debuglnD(charBuf);
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
  doTheOledThing(NODEID, RECEIVER, (int16_t)rf69.lastRssi(), current_millis, previous_millis);
}

// check to see if the reciever/robot is trying to send us a ping
void checkForPing() {
  uint8_t len = sizeof(buf);
  uint8_t from;
  if (rf69_manager.recvfromAck(buf, &len, &from)) {
    trafficOn();
    buf[len] = 0;
    debugD("Got msg from node "); debugD(from); debugD(": ");
    for (byte i = 0; i < len; i++) {
      debugD((char)buf[i]);
    }
    debugD("* ");
    if (strcmp((char *)&buf, (char *)&ping) != 0) {
      debuglnD("Not a ping.");
      statusError();
    } else {
      previous_millis = current_millis;      
      current_millis = millis(); 
      debuglnD("ok");
      statusOk();     
    }

    if (!rf69_manager.sendtoWait(ping_ack, sizeof(ping_ack), from)) {
      Serial.println("Sending failed (no ack)");
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
  //while (DEBUG && !Serial); // wait until serial console is open
  Serial.begin(SERIAL_BAUD);
  // Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  initStatus();
  initOled();
  initControls();
  
  // Initial State
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  debuglnD("Feather RFM69 Transmitter");

  // Hard Reset the RFM69 module
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  if (!rf69_manager.init()) {
    debuglnD("RFM69 radio init failed");
    statusError();
    while (1);
  }
  debuglnD("RFM69 radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  if (!rf69.setFrequency(RF69_FREQ)) {
    debuglnD("setFrequency failed");
    statusError();
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
  
  delay(2000); // Pause for 2 seconds
  char charBuf[100]; // for sprintf
  sprintf(charBuf, "Transmitting RFM69 radio @%.1f, Mhz", RF69_FREQ);
  debuglnD(charBuf);

  oled();
}

/* **************************
 * Continuous Loop
 * **************************/
void loop() {
  readJoystick();
  if (hasJoystickChanged()) {
    trafficOn();
    printJoystick();
    packData();
    sendData();
    copyJoystickData();
    trafficOff();
  } else {
    Serial.flush();
    //radio.sleep();
  }
  checkForPing();
}
