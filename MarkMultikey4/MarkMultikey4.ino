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
const long LEDblink1 = 1000;  
const long LEDblink2 = 500;
unsigned long previousMillis = 0;   
int led1State = LOW;   
int led2State = LOW;   
int led3State = LOW;   
const byte ROWS = 5; //five rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {

{'1','2','3'},
{'4','5','6'},
{'7','8','9'},
{'A','B','C'},
{'D','E','F'}
};

char keys2[ROWS][COLS] = {
{'A','B','C'},
{'D','E','F'},
{'1','2','3'},
{'4','5','6'},
{'7','8','9'}
};

byte rowPins[ROWS] = {8, 13, 12, 11, 6}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {5, 9, 10}; //connect to the column pinouts of the kpd

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

unsigned long loopCount;
unsigned long startTime;
String msg;

//LED Stuff
int octHi = 2; //Green LED
int octLo = 3; //Yellow LED
int deviceModeLED = 4; //Red LED

//Midi stuff
int pageMode = 0; // 0 is off, 1 is on
unsigned long deviceMode = 0; // 0 = note mode, 1 = channel mode, 2 = program change mode
int pageNum = 0; // this indicates the page number for notes
int buttonNum = 0; 
int channel = 0x90;
int velocity = 0x6F;
bool playNote = false;
int note;

int pitch( int x, int y ){ // x = pageNum; y = buttonNum
  int result = (x * 12) + y; 
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
void keyFunctions (char keyThatWasPressed, char keyState) {
  playNote = true;
  note = pageNum * 12;
  switch (keyThatWasPressed) {
    case '1':
    note = pageNum * 12;
    playNote = true;
    break;
    case '2':
    note = note + 1;
    playNote = true;
    break;
    case '3':
    note = note + 2;
    playNote = true;
    break;
    case '4':
    note = note + 3;
    playNote = true;
    break;
    case '5':
    note = note + 4;
    playNote = true;
    break;
    case '6':
    note = note + 5;
    playNote = true;
    break;
    case '7':
    note = note + 6;
    playNote = true;
    break;
    case '8':
    note = note + 7;
    playNote = true;
    break;
    case '9':
    note = note + 8;
    playNote = true;
    break;
    case 'A':
      note = note + 9;
      playNote = true;
      break;
    case 'B':
      note = note + 10;
      playNote = true;
      break;
    case 'C':
      note = note + 11;
      playNote = true;
      break;
    //Commented out to make room for Mode button
//    case 'D':
//    note = note + 12;
//    playNote = true;
//    break;
    case 'D':
      deviceMode ++;
    break;
    default:
      // if nothing else matches, do the default
      // default is optional
      break;
    case 'E':
      //note = pitch(pageNum, 14);
      if (keyState == PRESSED)
      {
        pageNum --;
      }
      playNote = false;
      break;
    case 'F':
      //note = pitch(pageNum, 15);
      if (keyState == PRESSED)
      {
        pageNum ++;
      }
      playNote = false;
      break;
  }
}


void setup() {
    //LED Setup
    pinMode(octHi, OUTPUT);
    pinMode(octLo, OUTPUT);
    pinMode(deviceModeLED, OUTPUT);
    Serial.begin(31250);
    loopCount = 0;
    startTime = millis();
    msg = "";
    pageNum = 5;
}



void loop() {
  //control the LEDs
  unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= LEDblink1) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
    }
    int currDeviceMode = deviceMode % 3;
    if (currDeviceMode == 0) {
      digitalWrite(deviceModeLED, LOW);  
    } else if (currDeviceMode == 1) {
      digitalWrite(deviceModeLED, HIGH); 
    } else if (currDeviceMode == 2) {
        if (led3State == LOW) {
          led3State = HIGH;
          digitalWrite(deviceModeLED, led3State);    
      } else {
          led3State = LOW;
      }
    }

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
                    //int note = pitch(pageNum, (kpd.key[i].kchar - 1));
              keyFunctions(kpd.key[i].kchar, kpd.key[i].kstate);
                      
                  if (playNote)
                  {
                    switch (kpd.key[i].kstate) // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
                    {  
                      case PRESSED:
                        noteOn(channel, note, velocity);
                        //note = pitch(pageNum, (kpd.key[i].kchar - 1));
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
