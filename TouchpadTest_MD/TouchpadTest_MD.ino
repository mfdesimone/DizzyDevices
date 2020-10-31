#include <Adafruit_MPR121.h>
// MPR121 stuff
#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif
#define touch_Thr 3000

// You can have up to 4 on one i2c bus but one is enough for  testing!
Adafruit_MPR121 cap = Adafruit_MPR121();
// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;




byte LH1;   
byte LH2;  
byte LH3;   
byte LH4;  
byte RH1;   
byte RH2;  
byte RH3;   
byte RH4;  
byte RH5;
byte RH6;
byte TH1;
byte TH2;
byte TH3;
byte TH4;
byte OCTup; // Octave switch key (pitch change +12) 
byte OCTdn; // Octave switch key (pitch change -12) 


void setup() {
  //MPR121 stuff
  pinMode(10, INPUT_PULLUP);
  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }
  
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 
  
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");

}

void loop() {
  readSwitches();
  LH4=0;
  RH5=0;
  RH1=0;
  RH6=0;
  TH1=0;
  TH2=0;
  TH3=0;
  TH4=0;
  delay(200);
}

void readSwitches(){
//  Serial.println(analogRead(A11));
  analogRead(A11); //breath sensor
  LH4=0;
  RH5=0;
  RH1=0;
  RH6=0;
  TH1=0;
  TH2=0;
  TH3=0;
  TH4=0;
  if (cap.touched() & (1 << 5)){LH4=1;} // this should be a true or false, or 1 or 0. The MPR121 board does the threshold stuff automatically, and just returns a 16 bit number when we use cap.touched(). The lower 12 bits of that number correspond to each of the 12 touchpads
  if (cap.touched() & (1 << 4)){RH5=1;}
  if (cap.touched() & (1 << 7)){RH1=1;}
  if (cap.touched() & (1 << 6)){RH6=1;}
  if (cap.touched() & (1 << 2)){TH1=1;}
  if (cap.touched() & (1 << 3)){TH2=1;}
  if (cap.touched() & (1 << 0)){TH3=1;}
  if (cap.touched() & (1 << 1)){TH4=1;}
  
  LH1=touchRead(17)>touch_Thr; //this will return true, or 1, if the touchRead measurement is greater than the touch_Thr, or false / 0 if it's lesser. Then, this 1 or 0 is used in the fingering equations below
  LH2=touchRead(0)>touch_Thr;
  LH3=touchRead(25)>touch_Thr;
  RH2=touchRead(22)>touch_Thr;
  RH3=touchRead(23)>touch_Thr;
  RH4=touchRead(1)>touch_Thr;
  OCTup=touchRead(15)>touch_Thr;
  OCTdn=touchRead(16)>touch_Thr;

  if(LH4){Serial.println("LH4");}
  if(RH5){Serial.println("RH5");}
  if(RH1){Serial.println("RH1");}
  if(RH6){Serial.println("RH6");}
  if(TH1){Serial.println("TH1");}
  if(TH2){Serial.println("TH2");}
  if(TH3){Serial.println("TH3");}
  if(TH4){Serial.println("TH4");}

  if(LH1){Serial.println("LH1");}
  if(LH2){Serial.println("LH2");}
  if(LH3){Serial.println("LH3");}
  if(RH2){Serial.println("RH2");}
  if(RH3){Serial.println("RH3");}
  if(RH4){Serial.println("RH4");}
  if(OCTup){Serial.println("OCTup");}
  if(OCTdn){Serial.println("OCTdn");}

}
