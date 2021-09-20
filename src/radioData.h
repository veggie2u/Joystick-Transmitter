#ifndef RADIO_DATA_H
#define RADIO_DATA_H

#define NETWORKID     100
#define RF69_FREQ 915.0
#define IS_RFM69HCW   true

/* for pins on M4 external and feather*/
#define RFM69_CS      10
#define RFM69_INT     7
#define RFM69_RST     9

enum Controls {
  redButton,
  greenButton,
  yellowButton,
  blueButton
};

//************************************************************************
//****** Real Bit Level Boolean Utilisation Macro Definitions  ***********
//****** Created by Trevor D.BEYDAG 09 May 2016                ***********
//****** http://lab.dejaworks.com                              ***********
//****** Please keep credits intact when using and/or reproducing ********
//************************************************************************
#define BB_TRUE(bp,bb)    bp |= bb
#define BB_FALSE(bp,bb)   bp &= ~(bb)
#define BB_READ(bp,bb)    bool(bp & bb)

#endif