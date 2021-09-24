#include <Arduino.h>

/*
  Joystick transmitter. Sends x/y and buttons using RF69 radio.
  Listens for a ping from reciever.
*/
#include <SPI.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>
#include <utils.h>
#include <debug.h>
#include <status.h>
#include <controls.h>
#include <screen.h>
#include "radioData.h"

#define NODEID        2    // The unique identifier of this node
#define RECEIVER      1    // The recipient of packets

#define SERIAL_BAUD   115200

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
  Serial.print("Sending ");
  Packet_Packed packedData = getPackedData();
  Serial.print(sizeof(packedData));
  Serial.print(" bytes - ");    
  if (rf69_manager.sendtoWait((uint8_t *)(&packedData), sizeof(packedData), RECEIVER)) {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (rf69_manager.recvfromAckTimeout(buf, &len, 2000, &from)) {
      buf[len] = 0; // zero out remaining string
      Serial.print("Got reply from #"); Serial.print(from);
      Serial.print(" [RSSI :");
      Serial.print(rf69.lastRssi());
      Serial.print("] : ");
      Serial.println((char*)buf);     
      statusOk();
      previous_millis = current_millis;      
      current_millis = millis();
    } else {
      Serial.println("No reply, is anyone listening?");
      statusError();
    }
  } else {
    Serial.println("Sending failed (no ack)");
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
    Serial.print("Got msg from node ");
    Serial.print(from); Serial.print(": ");
    for (byte i = 0; i < len; i++) {
      Serial.print((char)buf[i]);
    }
    Serial.print("* ");
    if (strcmp((char *)&buf, (char *)&ping) != 0) {
      Serial.println("Not a ping.");
      statusError();
    } else {
      previous_millis = current_millis;      
      current_millis = millis(); 
      Serial.println("ok");
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

  Serial.println("Feather RFM69 Transmitter");

  // Hard Reset the RFM69 module
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  if (!rf69_manager.init()) {
    Serial.println("RFM69 radio init failed");
    while (1);
    statusError();
  }
  Serial.println("RFM69 radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
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
  Serial.print("Transmitting "); Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");
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
    //Log.infoln("hello debug");
    debuglnD("hello debug");
  } else {
    Serial.flush();
    //radio.sleep();
  }
  checkForPing();
}
