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
const int LEDblink1 = 470;  
const int LEDblink2 = 5;
const int LEDblink3 = 10;

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

//char keys2[ROWS][COLS] = {
//{'A','B','C'},
//{'D','E','F'},
//{'1','2','3'},
//{'4','5','6'},
//{'7','8','9'}
//};

byte rowPins[ROWS] = {8, 13, 12, 11, 6}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {5, 9, 10}; //connect to the column pinouts of the kpd

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

unsigned long loopCount;
unsigned long startTime;
String msg;

//LED Stuff
int octHi = 3; //Green LED
int octLow = 4; //Yellow LED
int deviceModeLED = 2; //Red LED

//Midi stuff
int pageMode = 0; // 0 is off, 1 is on
int channelMode = 0;
int deviceMode = 0; // 0 = note mode, 1 = channel mode, 2 = program change mode
int modeSelect = 0; // 0 is off, 1 is on
int pageNum = 0; // this indicates the page number for notes
int buttonNum = 0; 
int channelOn = 0x90;
int channelOff = 0x00;
int velocity = 0x6F;
int velocityOff = 0x00;
bool playNote = false;
int note;

int pitch( int x, int y ){ // x = pageNum; y = buttonNum
  int result = (x * 12) + y; 
  return result; //let's calculate the pitch number to send. each page number * 12 is an octave
}

void noteEvent(int pitchNum) {
    //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
    noteOn(channelOn, pitchNum, velocity);
    //delay(1);
    //Note on channel 1 (0x90), some note value (note), silent velocity (0x00):
    noteOff(channelOff, pitchNum, velocityOff);
    //delay(1);
}
void keyFunctions (char keyThatWasPressed, char keyState) {
  
  if (keyThatWasPressed == 'D' && keyState == PRESSED){
    if (modeSelect == 0){
      playNote = false;
      modeSelect = 1;
     } 
  }
  //If we're in modeSelect mode
  if (modeSelect == 1 && keyState == PRESSED){
    
    switch (keyThatWasPressed) {
      case '1':
        playNote = false;
        deviceMode = 0;
        modeSelect = 0;
        break;
      case '2':
        deviceMode = 1;
        modeSelect = 0;
        break;
      case '3':
        deviceMode = 2;
        modeSelect = 0;
        break;
    }
  }
  if (modeSelect == 0 && keyState == PRESSED && deviceMode == 1){
    
    switch (keyThatWasPressed) {
      case '1':
        channelOn = 0x90;
        channelOff = 0x80;
        break;
      case '2':
        channelOn = 0x91;
        channelOff = 0x81;
        break;
      case '3':
        channelOn = 0x92;
        channelOff = 0x82;
        break;
      case '4':
        channelOn = 0x93;
        channelOff = 0x83;
        break;
      case '5':
        channelOn = 0x94;
        channelOff = 0x84;
        break;
      case '6':
        channelOn = 0x95;
        channelOff = 0x85;
        break;
      case '7':
        channelOn = 0x96;
        channelOff = 0x86;
        break;
      case '8':
        channelOn = 0x97;
        channelOff = 0x87;
        break;
      case '9':
        channelOn = 0x98;
        channelOff = 0x88;
        break;
      case 'A':
        channelOn = 0x99;
        channelOff = 0x89;
        break;
      case 'B':
        channelOn = 0x9A;
        channelOff = 0x8A;
        break;
      case 'C':
        channelOn = 0x9B;
        channelOff = 0x8B;
        break;
    }
    //deviceMode = 0;
  }
  
  note = pageNum * 12;
  //If we're not in modeSelect mode
  if (modeSelect == 0 && deviceMode == 0 ){
    playNote = true;
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
      default:
        // if nothing else matches, do the default
        // default is optional
        break;
    }
  }
}

void setup() {
    //LED Setup
    pinMode(octHi, OUTPUT);
    pinMode(octLow, OUTPUT);
    pinMode(deviceModeLED, OUTPUT);
    Serial.begin(31250);
    loopCount = 0;
    startTime = millis();
    msg = "";
    pageNum = 5;
}



void loop() {
  //control the LEDs
  if (modeSelect == 1) {
    digitalWrite(deviceModeLED, HIGH);  
  } else {
    digitalWrite(deviceModeLED, LOW); 
  }
  int blinkRate = LEDblink1 % loopCount;
  if (blinkRate < 12 && blinkRate > 1){
    digitalWrite(octHi, HIGH); 
  } else {
    digitalWrite(octHi, LOW); 
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
  if (kpd.getKeys()) {
      for (int i=0; i<LIST_MAX; i++) {  // Scan the whole key list.
      
          if ( kpd.key[i].stateChanged ) {  
            // Only find keys that have changed state. 
            //int note = pitch(pageNum, (kpd.key[i].kchar - 1));
            keyFunctions(kpd.key[i].kchar, kpd.key[i].kstate);
                    
                if (playNote && deviceMode == 0)
                {
                  switch (kpd.key[i].kstate) // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
                  {  
                    case PRESSED:
                      noteOn(channelOn, note, velocity);
                      //note = pitch(pageNum, (kpd.key[i].kchar - 1));
                      break;
                    case HOLD:
                      msg = " HOLD.";
                      break;
                    case RELEASED:
                      noteOff(channelOff, note, velocityOff);
                      break;
                    case IDLE:
                      msg = " IDLE.";
                  }
                }
          }
      }
  }
}






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
