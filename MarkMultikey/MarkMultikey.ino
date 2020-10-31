/* @file MultiKey.ino
|| @version 1.0
|| @author Mark Stanley
|| @contact mstanley@technologist.com
||
|| @description
|| | The latest version, 3.0, of the keypad library supports up to 10
|| | active keys all being pressed at the same time. This sketch is an
|| | example of how you can get multiple key presses from a keypad or
|| | keyboard.
|| #
*/

#include <Keypad.h>

const byte ROWS = 5; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {

{'1','2','3'},
{'4','5','6'},
{'7','8','9'},
{'*','0','#'},
{'R','H','M'}
};
byte rowPins[ROWS] = {2, 3, 4, 5, 6}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {9, 8, 7}; //connect to the column pinouts of the kpd

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

unsigned long loopCount;
unsigned long startTime;
String msg;


int pageMode = 0; // 0 is off, 1 is on
int pageNum = 0; // this indcates the page number for notes
int buttonNum = 0; 
int channel = 0x90;
int velocity = 0x45;
int pitch( int x, int y ){ // x = pageNum; y = buttonNum
  int result = x * 12 + y; 
  return result; //let's calculate the pitch number to send. each page number * 12 is an octave
}

void noteEvent(int pitchNum) {
    //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
    noteOn(channel, pitchNum, velocity);
    //delay(1);
    //Note on channel 1 (0x90), some note value (note), silent velocity (0x00):
    noteOff(channel, pitchNum, 0x00);
    //delay(1);
}

void setup() {
    Serial.begin(31250);
    loopCount = 0;
    startTime = millis();
    msg = "";
}


void loop() {
    loopCount++;
    if ( (millis()-startTime)>5000 ) {
        //Serial.print("Average loops per second = ");
        //Serial.println(loopCount/5);
        startTime = millis();
        loopCount = 0;
    }

    // Fills kpd.key[ ] array with up-to 10 active keys.
    // Returns true if there are ANY active keys.
    if (kpd.getKeys())
    {
        for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
        {
            if ( kpd.key[i].stateChanged ) 
            {  // Only find keys that have changed state. 
                if (kpd.key[i].kchar != 'R' || 'H' || 'M' || '*' || '#') 
                {
                    int note = pitch(pageNum, (kpd.key[i].kchar - 1));
                    switch (kpd.key[i].kstate) // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
                    {  
                        case PRESSED:
                        noteOn(channel, note, velocity);
                        note = pitch(pageNum, (kpd.key[i].kchar - 1));
                    break;
                        case HOLD:
                        msg = " HOLD.";
                    break;
                        case RELEASED:
                        noteOn(channel, note, 0x00);
                    break;
                        case IDLE:
                        msg = " IDLE.";
                    }
      //                Serial.print("Key ");
      //                Serial.print(kpd.key[i].kchar);
      //                Serial.println(msg);
                }
                else if (kpd.key[i].kchar == 'R' || 'H' || 'M')
                {
                    switch (kpd.key[i].kstate) // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
                    {  
                        case PRESSED:
                        msg = " PRESSED.";
                    break;
                        case HOLD:
                        msg = " HOLD.";
                    break;
                        case RELEASED:
                        msg = " RELEASED.";
                    break;
                        case IDLE:
                        msg = " IDLE.";
                    }
                }
            }
        }
    }
}  // End loop






void noteOff(int cmd, int pit, int vel) {
  Serial.write(cmd); //0x90 is a note on command, using channel 1. 0x91 is a note on command using channel 2
  Serial.write(pit); // pitch #, in hexadecimal
  Serial.write(vel); //sending a note on command with velocity zero is the same as a note off command
}
void noteOn(int cmd, int pit, int vel) {
  Serial.write(cmd);
  Serial.write(pit);
  Serial.write(vel); //0x45 is middle velocity
}
