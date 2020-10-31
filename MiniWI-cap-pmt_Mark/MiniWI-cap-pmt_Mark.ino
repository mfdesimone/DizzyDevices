#include <Wire.h>
#include <Adafruit_MPR121.h>

/*
NAME:                 MiniWI Cap Touch and Portamento ver.
WRITTEN BY:           JOHAN BERGLUND
CREDITS:              State machine from the Gordophone blog by GORDON GOOD
DATE:                 2016-06-01
FILE SAVED AS:        MiniWI-cap-pmt.ino
FOR:                  Arduino Pro Mini, ATmega328, version with breakouts for A6 and A7
CLOCK:                16.00 MHz CRYSTAL                                        
PROGRAMME FUNCTION:   Wind Controller with EWI style key setup (reduced) with optional Casio DH addition, 
                      Freescale MPX5010GP breath sensor, PS2 style thumb joysticks 
                      for octave selection and pb/mod control, capacitive touch keys, output to 5-pin DIN MIDI 

HARDWARE NOTES:
* For the MIDI connection, attach a MIDI out Female 180 Degree 5-Pin DIN socket to Arduino.
* Socket is seen from solder tags at rear.
* DIN-5 pinout is:                                         _______ 
*    pin 2 - GND                                          /       \
*    pin 4 - 220 ohm resistor to +5V                     | 1     3 |  MIDI jack
*    pin 5 - Arduino Pin 1 (TX) via a 220 ohm resistor   |  4   5  |
*    all other pins - unconnected                         \___2___/
*
* Left hand thumb joystick controls octaves.
* Up/down axis is connected to Arduino pin A6.
* 
*       +1  
*       ^
*     < o >
*       v
*      -1
* 
* A potentiometer connected to Arduino pin A7 sets the base octave -2 to +2 from startnote octave.
*
* Right hand thumb joystick controls pitch bend and modulation.
* Pitch bend and modulation are connected to Arduino pins A0 and A1,
* on DIP rows.
* 
*     PB up
*       ^
* Mod < o > Mod
*       v
*     PB dn
*     
* A potentiometer controls portamento speed setting.
* It is connected to Arduino pin A2.  
*     
* The Freescale MPX5010GP pressure sensor output (V OUT) is connected to Arduino pin A3.
* 
* Sensor pinout
* 1: V OUT (pin with indent)
* 2: GND
* 3: VCC (to 5V)    
* 4: n/c
* 5: n/c
* 6: n/c
*     
*     
* Adafruit MPR121 board connected to Arduino I2C ports (A4-SDA and A5-SCL on the Pro Mini)
* Touch keys including portamento key connected to MPR121 board.
* 
* Midi panic on pin 11 and 12 (internal pullup, both pins low sends all notes off)
* 
*/

//_______________________________________________________________________________________________ DECLARATIONS

#define ON_Thr 30       // Set threshold level before switching ON
#define ON_Delay   20   // Set Delay after ON threshold before velocity is checked (wait for tounging peak)
#define breath_max 300  // Blowing as hard as you can
//#define breath_max 1023  // Blowing as hard as you can
#define modsLo_Thr 411  // Low threshold for mod stick center
#define modsHi_Thr 611  // High threshold for mod stick center
#define octsLo_Thr 311  // Low threshold for octave stick center
#define octsHi_Thr 711  // High threshold for octave stick center
#define octsLo1_Thr 409  // Low threshold for octave select pot
#define octsHi1_Thr 613  // High threshold for octave select pot
#define octsLo2_Thr 205  // Low threshold 2 for octave select pot
#define octsHi2_Thr 818  // High threshold 2 for octave select pot
#define PB_sens 4095    // Pitch Bend sensitivity 0 to 8191 where 8191 is full pb range
#define casioMod 0      // Default selection on/off for Casio DH 2nd octave fingering (LH1 lifted)

// The three states of our state machine

// No note is sounding
#define NOTE_OFF 1

// We've observed a transition from below to above the
// threshold value. We wait a while to see how fast the
// breath velocity is increasing
#define RISE_WAIT 2

// A note is sounding
#define NOTE_ON 3

// Send CC data no more than every CC_INTERVAL
// milliseconds
#define CC_INTERVAL 15 


//variables setup

byte casiomodSelect;               // Change the fingering setting w octave stick up at power on

int state;                         // The state of the state machine
unsigned long ccSendTime = 0L;     // The last time we sent CC values
unsigned long breath_on_time = 0L; // Time when breath sensor value went over the ON threshold
int initial_breath_value;          // The breath value at the time we observed the transition

long lastDebounceTime = 0;         // The last time the fingering was changed
long debounceDelay = 30;           // The debounce time; increase if the output flickers
int lastFingering = 0;             // Keep the last fingering value for debouncing

byte MIDIchannel=0;                // MIDI channel 1

int modLevel;
int oldmod=0;

int pitchBend;
int oldpb=8192;

int portLevel;
int oldport=-1;

int breathLevel=0;   // breath level (smoothed) not mapped to CC value

int pressureSensor;  // pressure data from breath sensor, for midi breath cc and breath threshold checks
byte velocity;       // remapped midi velocity from breath sensor

int fingeredNote;    // note calculated from fingering (switches) and octave joystick position
byte activeNote;     // note playing
//byte startNote=73;   // set startNote to C# (change this value in steps of 12 to start in other octaves)
byte startNote=60;   // set startNote to C# (change this value in steps of 12 to start in other octaves)

byte midistatus=0;
byte x;
byte LedPin = 13;    // select the pin for the LED

Adafruit_MPR121 touchSensor = Adafruit_MPR121(); // This is the 12-input touch sensor

            // Key variables, TRUE (1) for pressed, FALSE (0) for not pressed
byte LH1;   // Left Hand key 1 (pitch change -2) 
byte LH2;   // Left Hand key 2  (with LH1 also pressed pitch change is -2, otherwise -1)
byte LH3;   // Left Hand key 3 (pitch change -2)
byte LH4;   // Left Hand bis key (pitch change -1 unless both LH1 and LH2 are pressed)
//byte LHp1;  // Left Hand pinky key 1 (pitch change +1)
//byte LHp2=0;  // Left Hand pinky key 2 (pitch change -1) --- Not used in this version
//byte RHs=0;   // Right Hand side key  (pitch change -2 unless LHp1 is pressed) --- Not used in this version
byte RH1;   // Right Hand key 1 (with LH3 also pressed pitch change is -2, otherwise -1)
byte RH2;   // Right Hand key 2 (pitch change -1)
byte RH3;   // Right Hand key 3 (pitch change -2)
byte RH4;   // Right Hand key 3 (pitch change -2)
//byte RHp1=0;  // Right Hand pinky key 1 (pitch change +1) --- Not used in this version
//byte RHp2;  // Right Hand pinky key 2 (pitch change -1)
//byte RHp3;  // Right Hand pinky key 3 (pitch change -2)
byte OCTup=0; // Octave switch key (pitch change +12) --- Not used in this version 
byte OCTdn=0; // Octave switch key (pitch change -12) --- Not used in this version

byte PortK;   // Portamento momentary on switch
byte oldportk;

int joyOct; // Octave shifting by joystick or potentiometer

//_______________________________________________________________________________________________ SETUP

void setup() {

  state = NOTE_OFF;  // initialize state machine
  
  pinMode(LedPin,OUTPUT);   // declare the LED's pin as output

  // joystick buttons for midi panic
  pinMode(11,INPUT_PULLUP); // panic pin 1/2
  pinMode(12,INPUT_PULLUP); // panic pin 2/2

  // Set up touch sensor
  if (!touchSensor.begin(0x5A)) {
    while (1);  // Touch sensor initialization failed - stop doing stuff
  }
  // Set the selection for Casio fingering - Pitch stick up at power on changes from default
  if (analogRead(A6) > octsHi_Thr) {
    casiomodSelect=!casioMod;
  } else{
    casiomodSelect=casioMod;
  }
  for (x=1; x<=4; x++){  // Do the flashy-flashy to say we are up and running
    digitalWrite( LedPin, HIGH );
    delay(300);
    digitalWrite( LedPin, LOW );
    delay(300);
  }

  Serial.begin(31250);  // start serial with midi baudrate 31250
  Serial.flush();
}

//_______________________________________________________________________________________________ MAIN LOOP

void loop() {

  // if both joystick buttons are pressed, send all notes off
  if ((digitalRead(11) == 0) && (digitalRead(12) == 0)){
    midiPanic();
  }
  
  pressureSensor = analogRead(A9); // Get the pressure sensor reading from analog pin A3

  if (state == NOTE_OFF) {
    if (pressureSensor > ON_Thr) {
      // Value has risen above threshold. Move to the ON_Delay
      // state. Record time and initial breath value.
      breath_on_time = millis();
      initial_breath_value = pressureSensor;
      state = RISE_WAIT;  // Go to next state
    }
  } else if (state == RISE_WAIT) {
    if (pressureSensor > ON_Thr) {
      // Has enough time passed for us to collect our second
      // sample?
      if (millis() - breath_on_time > ON_Delay) {
        // Yes, so calculate MIDI note and velocity, then send a note on event
        readSwitches();
//        readOctaves();
        oldportk=2; // Set oldportk to a value other than 1 or 0 to make sure it always sends the data for new notes
        portamento();
        // We should be at tonguing peak, so set velocity based on current pressureSensor value        
        // If initial value is greater than value after delay, go with initial value, constrain input to keep mapped output within 7 to 127
        velocity = map(constrain(max(pressureSensor,initial_breath_value),ON_Thr,breath_max),ON_Thr,breath_max,7,127);
        breathLevel=constrain(max(pressureSensor,initial_breath_value),ON_Thr,breath_max);
        midiSend((0x90 | MIDIchannel), fingeredNote, velocity); // send Note On message for new note
        activeNote=fingeredNote;
        state = NOTE_ON;
      }
    } else {
      // Value fell below threshold before ON_Delay passed. Return to
      // NOTE_OFF state (e.g. we're ignoring a short blip of breath)
      state = NOTE_OFF;
    }
  } else if (state == NOTE_ON) {
    if (pressureSensor < ON_Thr) {
      // Value has fallen below threshold - turn the note off
      midiSend((0x80 | MIDIchannel), activeNote, velocity); //  send Note Off message 
      breathLevel=0;
      state = NOTE_OFF;
    } else {
      // Is it time to send more CC data?
      if (millis() - ccSendTime > CC_INTERVAL) {
         // deal with Breath, Pitch Bend and Modulation
         breath();
         pitch_bend();
         modulation();
         ccSendTime = millis();
      }    
      readSwitches();
//      readOctaves();
      if (fingeredNote != lastFingering){ //
        // reset the debouncing timer
        lastDebounceTime = millis();
      }
      if ((millis() - lastDebounceTime) > debounceDelay) {
      // whatever the reading is at, it's been there for longer
      // than the debounce delay, so take it as the actual current state
        if (fingeredNote != activeNote) {
          // Player has moved to a new fingering while still blowing.
          // Send a note off for the current note and a note on for
          // the new note.
          portamento();        
          velocity = map(constrain(pressureSensor,ON_Thr,breath_max),ON_Thr,breath_max,7,127); // set new velocity value based on current pressure sensor level
          midiSend((0x90 | MIDIchannel), fingeredNote, velocity); // send Note On message for new note
          midiSend((0x80 | MIDIchannel), activeNote, 0); // send Note Off message for previous note (legato)
          activeNote=fingeredNote;
        }
      }
    }
  }
  lastFingering=fingeredNote; 
}
//_______________________________________________________________________________________________ FUNCTIONS

//  Send a three byte midi message  
  void midiSend(byte midistatus, byte data1, byte data2) {
  digitalWrite(LedPin,HIGH);  // indicate we're sending MIDI data
  Serial.write(midistatus);
  Serial.write(data1);
  Serial.write(data2);
  digitalWrite(LedPin,LOW);  // indicate we're sending MIDI data
}

//**************************************************************

void midiPanic(){
  for (int i = 0; i < 128; i++){
    midiSend((0x80 | MIDIchannel), i, 0);
  }
}

//**************************************************************

void pitch_bend(){
  int pitchLSB;
  int pitchMSB;
  pitchBend = analogRead(A0); // read voltage on analog pin A0
  if (pitchBend > modsHi_Thr){
    pitchBend = map(pitchBend,modsHi_Thr,1023,8192,(8192 + PB_sens)); // go from 8192 to 16383 (full pb up) when off center threshold going up
  } else if (pitchBend < modsLo_Thr){
    pitchBend = map(pitchBend,0,modsLo_Thr,(8191 - PB_sens),8192); // go from 8192 to 0 (full pb dn) when off center threshold going down
  } else {
    pitchBend = 8192; // 8192 is 0 pitch bend
  }
  if (pitchBend != oldpb){// only send midi data if pitch bend has changed from previous value
    pitchLSB = pitchBend & 0x007F;
    pitchMSB = (pitchBend >>7) & 0x007F; 
    midiSend((0xE0 | MIDIchannel), pitchLSB, pitchMSB);
    oldpb=pitchBend;
  }
}

//***********************************************************

void modulation(){
  modLevel = analogRead(A1); // read voltage on analog pin A1
  if (modLevel > modsHi_Thr){
    modLevel = map(modLevel,modsHi_Thr,1023,0,127); // go from 0 to full modulation when off center threshold going right(?)
  } else if (modLevel < modsLo_Thr){
    modLevel = map(modLevel,0,modsLo_Thr,127,0); // go from 0 to full modulation when off center threshold going left(?)
  } else {
    modLevel = 0; // zero modulation in center position
  }
  if (modLevel != oldmod){  // only send midi data if modulation has changed from previous value
    midiSend((0xB0 | MIDIchannel), 1, modLevel);
    oldmod=modLevel;
  }
}

//***********************************************************

void portamento(){
  portLevel = map(analogRead(A2),0,1023,0,127); // read voltage on analog pin A7 and map to midi value
  if (portLevel != oldport){  // only send midi data if level has changed from previous value
    midiSend((0xB0 | MIDIchannel), 5, portLevel);   
    oldport=portLevel;
  }
  if (PortK != oldportk){  // only send midi data if status has changed from previous value
    if (PortK){ 
      midiSend((0xB0 | MIDIchannel), 65, 127); // send portamento on
    }
    else {
      midiSend((0xB0 | MIDIchannel), 65, 0); // send portamento off   
    }
    oldportk=PortK;
  }
}

//***********************************************************

void breath(){
  int breathCC;
  breathLevel = breathLevel*0.8+pressureSensor*0.2; // smoothing of breathLevel value
  breathCC = map(constrain(breathLevel,ON_Thr,breath_max),ON_Thr,breath_max,0,127);
  midiSend((0xB0 | MIDIchannel), 2, breathCC);
}
//***********************************************************

//void readOctaves(){
//  // Read octave joystick and set octave of the fingered note (run after readSwitches)
//  int xOctaves;
//  int yOctaves;
//  xOctaves = analogRead(A6); // read voltage on analog pin A6
//  yOctaves = analogRead(A7); // read voltage on analog pin A7 (this is now a separate potentiometer, not joystick axis)
//  joyOct = 0;
//  // xOctaves is up/down and the only used octave joystick direction in this version
//  if (xOctaves > octsHi_Thr) {
//    joyOct++;
//  } else if (xOctaves < octsLo_Thr) {
//    joyOct--;
//  }
//  // yOctaves in this version is a separate potentiometer setting base octave -2 to +2
//  if (yOctaves > octsHi1_Thr) {
//    joyOct++; // ++ or -- depending on joystick orientation
//  } else if (yOctaves < octsLo1_Thr) {
//    joyOct--; // ++ or -- depending on joystick orientation
//  }
//  if (yOctaves > octsHi2_Thr) {
//    joyOct++; // ++ or -- depending on joystick orientation
//  } else if (yOctaves < octsLo2_Thr) {
//    joyOct--; // ++ or -- depending on joystick orientation
//  }
//  //calculate midi note number from octave shifts
//  fingeredNote=fingeredNote+joyOct*12;
//}
//***********************************************************

void readSwitches(){  
  // Read switches and put value in variables
  uint16_t touchValue = touchSensor.touched();
  LH1=((touchValue >> 5) & 0x01);
  LH2=((touchValue >> 4) & 0x01);
  LH3=((touchValue >> 3) & 0x01);
  LH4=((touchValue >> 2) & 0x01);
  RH1=((touchValue >> 6) & 0x01);
  RH2=((touchValue >> 7) & 0x01);
  RH3=((touchValue >> 8) & 0x01);
  RH4=((touchValue >> 9) & 0x01);
  OCTup=((touchValue >> 1) & 0x01);
  OCTdn=((touchValue >> 0) & 0x01);
  //PortK=((touchValue >> 5) & 0x01); // portamento key
  //calculate midi note number from pressed keys
  if (casiomodSelect){
    fingeredNote=startNote - (2*LH1 + 4*LH2 + LH3 + 7*LH4)+(2*RH1 + 4*RH2 + RH3 + 7*RH4) +12*OCTup -12*OCTdn;
  } else {
    fingeredNote=startNote - (2*LH1 + 4*LH2 + LH3 + 7*LH4)+(2*RH1 + 4*RH2 + RH3 + 7*RH4) +12*OCTup -12*OCTdn;
  }
}
