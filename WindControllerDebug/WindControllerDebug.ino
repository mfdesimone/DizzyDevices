

#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
//#include "Filters.h"  // for the breath signal LP filtering, https://github.com/edgar-bonet/Filters
#include <Bounce2.h>



// MPR121 stuff
#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;
int touch_Thr = 3400;
  
void setup() {

  // put your setup code here, to run once:
//  analogReadResolution(12);   // set resolution of ADCs to 12 bit
}

void loop() {
  // put your main code here, to run repeatedly:
//int pressureSensor = (analogRead(A11));

 if (cap.touched()){
    Serial.println("fart");
};
}
