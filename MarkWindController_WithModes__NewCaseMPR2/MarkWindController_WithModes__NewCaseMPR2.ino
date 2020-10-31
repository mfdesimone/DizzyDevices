#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include "Filters.h"  // for the breath signal LP filtering, https://github.com/edgar-bonet/Filters
#include <Bounce2.h>

/*
NAME:                 T.WI
WRITTEN BY:           JOHAN BERGLUND
DATE:                 2017-10-17
FILE SAVED AS:        T.WI.ino
FOR:                  PJRC Teensy LC and Teensyduino
CLOCK:                48.00 MHz                                       
PROGRAMME FUNCTION:   Woodwind Controller using a Freescale MPX5010GP breath sensor,
                      a PSP1000 joystick and capacitive touch keys. Output to USB MIDI.  
HARDWARE NOTES:
    
* The Freescale MPX5010GP pressure sensor output (V OUT) is connected to pin 21(A7).
* (Warning: no voltage limiting before input, can harm Teensy if excessive pressure is applied.)
* 
* Sensor pinout
* 1: V OUT (pin with indent)
* 2: GND (to GND pin of Teensy)
* 3: VCC (to 5V pin of Teensy)    
* 4: n/c
* 5: n/c
* 6: n/c
*     
* Touch sensors are using the Teensy LC built in touchRead function.
* Electrodes connect directly to Teensy pins.
* 
* PSP style thumb slide joystick controls pitch bend and modulation.
* Pitch bend and modulation are connected to pins A6 and A0.
* Connections on joystick, bottom view with connectors towards you, L to R:
* 1: to VCC 3.3V
* 2: X (or Y depending on orientation) to analog input
* 3: to GND
* 4: Y (or X depending on orientation) to analog input
* 
*     PB up
*       ^
* Mod < o > Glide
*       v
*     PB dn
*   
*/

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

//_______________________________________________________________________________________________ DECLARATIONS

#define touch_Thr 2000  // threshold for Teensy touchRead, 1300-1800
#define ON_Thr 3600      // Set threshold level before switching ON
#define ON_Delay   20   // Set Delay after ON threshold before velocity is checked (wait for tounging peak)
#define breath_max 4095 // Threshold for maximum breath
#define modsLo_Thr 2600 // Low threshold for mod stick center
#define modsHi_Thr 3400 // High threshold for mod stick center
#define modsMin 1130     // PSP joystick min value
#define modsMax 4096    // PSP joystick max value
#define PB_sen1 8192    // Pitch Bend sensitivity 0 to 8192 where 8192 is full pb range, 4096 is half range
#define PB_sen2 683     // Selectable 1/12 PB sense for synths w fixed 12 semitones pb range
#define CCN_Port 5      // Controller number for portamento level
#define CCN_PortOnOff 65// Controller number for portamento on/off
#define portaMax  30    // 1 to 127, max portamento level

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
#define CC_INTERVAL 50

// EEPROM addresses for settings
#define VERSION_ADDR 0
#define TRANS1_ADDR 14
#define MIDI_ADDR 16
#define BREATH_CC_ADDR 18
#define BREATH_AT_ADDR 20
#define PORTAM_ADDR 24
#define PB_ADDR 26
#define EXTRA_ADDR 28
#define TRANS2_ADDR 40
#define BREATHCURVE_ADDR 44

//"factory" values for settings
#define VERSION 4
#define MIDI_FACTORY 1            // 1-16
#define BREATH_CC_FACTORY 2       // thats CC#2, see ccList
#define BREATH_AT_FACTORY 0       // aftertouch default off
#define PORTAM_FACTORY 2          // 0 - OFF, 1 - ON, 2 - SW
#define PB_FACTORY 0              // 0 - 1/2, 1 - 1/12
#define EXTRA_FACTORY 0           // 0 - Modulation wheel, 1 - Pitch Bend Vibrato
#define BREATHCURVE_FACTORY 2     // 0 to 12 (-4 to +4, S1 to S4)
#define TRANS1_FACTORY 0          // 1 - +2 semitones (C to D, F to G)
#define TRANS2_FACTORY 0          // 1 - -7 semitones (C to F, D to G) "alto mode"

#define maxSamplesNum 120

//variables setup


static int waveformsTable[maxSamplesNum] = {
  // Sine wave
  0x7ff, 0x86a, 0x8d5, 0x93f, 0x9a9, 0xa11, 0xa78, 0xadd, 0xb40, 0xba1,
  0xbff, 0xc5a, 0xcb2, 0xd08, 0xd59, 0xda7, 0xdf1, 0xe36, 0xe77, 0xeb4,
  0xeec, 0xf1f, 0xf4d, 0xf77, 0xf9a, 0xfb9, 0xfd2, 0xfe5, 0xff3, 0xffc,
  0xfff, 0xffc, 0xff3, 0xfe5, 0xfd2, 0xfb9, 0xf9a, 0xf77, 0xf4d, 0xf1f,
  0xeec, 0xeb4, 0xe77, 0xe36, 0xdf1, 0xda7, 0xd59, 0xd08, 0xcb2, 0xc5a,
  0xbff, 0xba1, 0xb40, 0xadd, 0xa78, 0xa11, 0x9a9, 0x93f, 0x8d5, 0x86a,
  0x7ff, 0x794, 0x729, 0x6bf, 0x655, 0x5ed, 0x586, 0x521, 0x4be, 0x45d,
  0x3ff, 0x3a4, 0x34c, 0x2f6, 0x2a5, 0x257, 0x20d, 0x1c8, 0x187, 0x14a,
  0x112, 0xdf, 0xb1, 0x87, 0x64, 0x45, 0x2c, 0x19, 0xb, 0x2,
  0x0, 0x2, 0xb, 0x19, 0x2c, 0x45, 0x64, 0x87, 0xb1, 0xdf,
  0x112, 0x14a, 0x187, 0x1c8, 0x20d, 0x257, 0x2a5, 0x2f6, 0x34c, 0x3a4,
  0x3ff, 0x45d, 0x4be, 0x521, 0x586, 0x5ed, 0x655, 0x6bf, 0x729, 0x794
};


int state;                         // The state of the state machine
unsigned long ccSendTime = 0L;     // The last time we sent CC values
unsigned long breath_on_time = 0L; // Time when breath sensor value went over the ON threshold
int initial_breath_value;          // The breath value at the time we observed the transition

unsigned long lastDebounceTime = 0;         // The last time the fingering was changed
unsigned long debounceDelay = 1;           // The debounce time; increase if the output flickers
int lastFingering = 0;             // Keep the last fingering value for debouncing
int lastFingeringL = 0;

byte MIDIchannel=1;                // MIDI channel 1

unsigned short breathCC;           // OFF:MW:BR:VL:EX:MW+:BR+:VL+:EX+
unsigned short breathAT;
unsigned short portamento;         // switching on cc65? just cc5 enabled? SW:ON:OFF
unsigned short curve;              // selected curve
unsigned short PB;
unsigned short mod;
unsigned short trans1;
unsigned short trans2;

byte ccList[9] = {0,1,2,7,11,1,2,7,11};  // OFF, Modulation, Breath, Volume, Expression,(then same sent in hires)

int breathLevel=0;   // breath level (smoothed) not mapped to CC value
int oldbreath=0;
unsigned int oldbreathhires=0;
unsigned int breathValHires=0;

byte portIsOn=0;     // keep track and make sure we send CC with 0 value when off threshold
int oldport=0;
bool monoIsOn = 0;
int pressureSensor;  // pressure data from breath sensor, for midi breath cc and breath threshold checks
byte velocitySend;   // remapped midi velocity from breath sensor

int modLevel;
int oldmod=0;
int lfoDepth=2;
int lfoLevel=0;
int lfo=0;

int pitchBend;
int oldpb=8192;
int oldcc16=0;
int PB_sens;
int modCCnumber = 1;
int octave = 0; //MD
int transposition = 0; //MD
int fingeredNote;    // note calculated from fingering (switches) and octave joystick position
int fingeredNoteL;
byte activeNote;     // note playing
byte activeNoteL;
byte startNote = 60; //MD
float filterFreq = 30.0; 

            // Key variables, TRUE (1) for pressed, FALSE (0) for not pressed
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
int cc16pad =0;

unsigned int curveM4[] = {0,4300,7000,8700,9900,10950,11900,12600,13300,13900,14500,15000,15450,15700,16000,16250,16383};
unsigned int curveM3[] = {0,2900,5100,6650,8200,9500,10550,11500,12300,13100,13800,14450,14950,15350,15750,16150,16383};
unsigned int curveM2[] = {0,2000,3600,5000,6450,7850,9000,10100,11100,12100,12900,13700,14400,14950,15500,16000,16383};
unsigned int curveM1[] = {0,1400,2850,4100,5300,6450,7600,8700,9800,10750,11650,12600,13350,14150,14950,15650,16838};
unsigned int curveIn[] = {0,1023,2047,3071,4095,5119,6143,7167,8191,9215,10239,11263,12287,13311,14335,15359,16383};
unsigned int curveP1[] = {0,600,1350,2150,2900,3800,4700,5600,6650,7700,8800,9900,11100,12300,13500,14850,16838};
unsigned int curveP2[] = {0,400,800,1300,2000,2650,3500,4300,5300,6250,7400,8500,9600,11050,12400,14100,16383};
unsigned int curveP3[] = {0,200,500,900,1300,1800,2350,3100,3800,4600,5550,6550,8000,9500,11250,13400,16383};
unsigned int curveP4[] = {0,100,200,400,700,1050,1500,1950,2550,3200,4000,4900,6050,7500,9300,12100,16282};
unsigned int curveS1[] = {0,600,1350,2150,2900,3800,4700,6000,8700,11000,12400,13400,14300,14950,15500,16000,16383};
unsigned int curveS2[] = {0,600,1350,2150,2900,4000,6100,9000,11000,12100,12900,13700,14400,14950,15500,16000,16383};
unsigned int curveS3[] = {0,600,1350,2300,3800,6200,8700,10200,11100,12100,12900,13700,14400,14950,15500,16000,16383};
unsigned int curveS4[] = {0,600,1700,4000,6600,8550,9700,10550,11400,12200,12900,13700,14400,14950,15500,16000,16383};

//_______________________________________________________________________________________________ SETUP


//init stuff for neopixel ring

#define PIN        6 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 12
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


#define OctUpPin 5
#define OctDnPin 4
#define TransUpPin 7
#define TransDnPin 8
#define MidChUpPin 2
#define MidChDnPin 3
int debounceInterval = 5;
bool OctUpState = 0;
bool OctDnState = 0;
bool TransUpState = 0;
bool TransDnState = 0;
bool MidChUpState = 0;
bool MidChDnState = 0;
Bounce OctUpDebo = Bounce();// Instantiate a Bounce object
Bounce OctDnDebo = Bounce();
Bounce TransUpDebo = Bounce();
Bounce TransDnDebo = Bounce();
Bounce MidChUpDebo = Bounce();
Bounce MidChDnDebo = Bounce();



//LED variables
int pitchClass = 0;
int pixelBrightnessMap[12] = {0,0,0,0,0,0,0,0,0,0,0,0}; // this is an array of 12 integers for the pixel's Brightness (HSB)
int pixelHueMap[12] = {0,0,0,0,0,0,0,0,0,0,0,0}; // this is an array of 12 integers for the pixel's Hue (HSB)


//mode setup
unsigned long instrumentModeCounter = 0;
int numberOfModes = 6;
int instrumentMode = 0;
//0 is normal melody mode. 
//1 is harmony mode, where right hand chooses main pitch #, and left hand adds a second pitch, at x # of half steps above 
//2 is duet mode, where each hand individually chooses a pitch #
//3 is bass mode. Like duet mode, except left hand is one or two octaves below melody. Octave switcher here only affects right hand pitch.

void setup() {
  //MPR121 stuff
//    Serial.begin(9600);
//
//  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
//    delay(10);
//  }
  
//  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 
//  
//  // Default address is 0x5A, if tied to 3.3V its 0x5B
//  // If tied to SDA its 0x5C and if SCL then 0x5D
//  if (!cap.begin(0x5A)) {
//    Serial.println("MPR121 not found, check wiring?");
//    while (1);
//  }
//  Serial.println("MPR121 found!");
  
  // Buttons & debouncing
  pinMode(MidChUpPin,INPUT_PULLUP); // Midi Channel +
  pinMode(MidChDnPin,INPUT_PULLUP); // Midi channel -
  pinMode(OctUpPin,INPUT_PULLUP); // octave up
  pinMode(OctDnPin,INPUT_PULLUP); // octave down
  pinMode(TransUpPin,INPUT_PULLUP); // transpose +
  pinMode(TransDnPin,INPUT_PULLUP); // transpose -
  OctUpDebo.attach(OctUpPin); // After setting up the button, setup the Bounce instance
  OctUpDebo.interval(debounceInterval); // interval in ms
  OctDnDebo.attach(OctDnPin);
  OctDnDebo.interval(debounceInterval); // interval in ms
  TransUpDebo.attach(TransUpPin);
  TransUpDebo.interval(debounceInterval); // interval in ms
  TransDnDebo.attach(TransDnPin);
  TransDnDebo.interval(debounceInterval); // interval in ms
  MidChUpDebo.attach(MidChUpPin);
  MidChUpDebo.interval(debounceInterval); // interval in ms
  MidChDnDebo.attach(MidChDnPin);
  MidChDnDebo.interval(debounceInterval); // interval in ms

  
  analogReadResolution(12);   // set resolution of ADCs to 12 bit
  state = NOTE_OFF;           // initialize state machine
  pinMode(13,OUTPUT);         // use Teensy LED for breath on indication
  digitalWrite(13,LOW);  
  
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  
  // if stored settings are not for current version, they are replaced by factory settings
  if (readSetting(VERSION_ADDR) != VERSION){
    writeSetting(VERSION_ADDR,VERSION);
    writeSetting(MIDI_ADDR,MIDI_FACTORY);
    writeSetting(TRANS1_ADDR,TRANS1_FACTORY);
    writeSetting(TRANS2_ADDR,TRANS2_FACTORY);
    writeSetting(BREATH_CC_ADDR,BREATH_CC_FACTORY);
    writeSetting(BREATH_AT_ADDR,BREATH_AT_FACTORY);
    writeSetting(PORTAM_ADDR,PORTAM_FACTORY);
    writeSetting(PB_ADDR,PB_FACTORY);
    writeSetting(EXTRA_ADDR,EXTRA_FACTORY);
    writeSetting(BREATHCURVE_ADDR,BREATHCURVE_FACTORY);
  }
  // read settings from EEPROM
  MIDIchannel  = readSetting(MIDI_ADDR);
  trans1       = readSetting(TRANS1_ADDR);
  trans2       = readSetting(TRANS2_ADDR);
  //breathCC     = readSetting(BREATH_CC_ADDR);
  breathCC     = 0x02; //0x02 is standard breath control CC
  //breathCC     = 0x0B; //0x0B is good for Session Horns velocity
  breathAT     = readSetting(BREATH_AT_ADDR);
  portamento   = readSetting(PORTAM_ADDR);
  PB           = readSetting(PB_ADDR);
  mod          = readSetting(EXTRA_ADDR);
  curve        = readSetting(BREATHCURVE_ADDR);

//  instrumentSettings(); // commented out because it seems to conflict with MPR121

//  if (PB) PB_sens = PB_sen2; else PB_sens = PB_sen1;
  PB_sens = PB_sen1;
  if (trans1) startNote += 2;
  if (trans2) startNote -= 7;
}

//_______________________________________________________________________________________________ MAIN LOOP

void loop() {
  if (!monoIsOn){
    usbMIDI.sendControlChange(126, 2, MIDIchannel);
    usbMIDI.sendControlChange(126, 2, (MIDIchannel +1));
    monoIsOn = true;
  }
  mainLoop();
}


void mainLoop() {  

  FilterOnePole breathFilter( LOWPASS, filterFreq );   // create a one pole (RC) lowpass filter
  while(1){


  
    // Update the Bounce instances :
    OctUpDebo.update();
    OctDnDebo.update();
    TransUpDebo.update();
    TransDnDebo.update();
    MidChUpDebo.update();
    MidChDnDebo.update();
    
    if (OctUpDebo.fell()){
      octave ++;  // octave up
    }
    if (OctDnDebo.fell()){
      octave --;  // octave down
    }
    if (TransUpDebo.fell()){
      transposition ++;  // transpose up half step
    }
    if (TransDnDebo.fell()){
      transposition --;  // transpose down half step
     }
    if (MidChUpDebo.fell()){ //Sustain pedal down
      usbMIDI.sendControlChange(64, 127, MIDIchannel);
      if (instrumentMode <2){
        usbMIDI.sendControlChange(64, 127, (MIDIchannel+1));
      }
    }
    if (MidChUpDebo.rose()){ // Sustain pedal up
      usbMIDI.sendControlChange(64, 0, MIDIchannel);
      if (instrumentMode <2){
        usbMIDI.sendControlChange(64, 0, (MIDIchannel+1));
      }
    }
    if (MidChDnDebo.fell()){ //using Midi channel down for mode selection at the moment
      instrumentModeCounter++;
      modeSelector();
      pixels.clear();
//      Serial.println(instrumentModeCounter);
    }
    
  
    breathFilter.input(analogRead(A7));
    Serial.println(analogRead(A7));
    pressureSensor = constrain((int)breathFilter.output(),3400,4095); // Get the filtered pressure sensor reading from analog pin A7, input from sensor MP3V5004GP
  
    if (state == NOTE_OFF) {
      if (pressureSensor > ON_Thr) {
        // Value has risen above threshold. Move to the ON_Delay
        // state. Record time and initial breath value.
        breath_on_time = millis();
        initial_breath_value = pressureSensor;
        state = RISE_WAIT;  // Go to next state
        readSwitches();
      }
    } else if (state == RISE_WAIT) {
      if (pressureSensor > ON_Thr) {
        
        // Has enough time passed for us to collect our second
        // sample?
        if (millis() - breath_on_time > ON_Delay) {
          // Yes, so calculate MIDI note and velocity, then send a note on event
//          readSwitches();
          // We should be at tonguing peak, so set velocity based on current pressureSensor value        
          // If initial value is greater than value after delay, go with initial value, constrain input to keep mapped output within 1 to 127
          breathLevel=constrain(max(pressureSensor,initial_breath_value),ON_Thr,breath_max);
          breathValHires = breathCurve(map(constrain(breathLevel,ON_Thr,breath_max),ON_Thr,breath_max,0,16383));
          velocitySend = (breathValHires >>7) & 0x007F;
          velocitySend = constrain(velocitySend,1,127);
          breath(); // send breath data
          usbMIDI.sendNoteOn(fingeredNote, velocitySend, MIDIchannel); // send Note On message for new note 
          Serial.println(fingeredNote);
          if (instrumentMode <2){
            usbMIDI.sendNoteOn(fingeredNoteL, velocitySend, (MIDIchannel +1) ); // send Note On message for new note 
          }
          digitalWrite(13,HIGH);
          activeNote=fingeredNote;
          activeNoteL=fingeredNoteL;
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
        usbMIDI.sendNoteOff(activeNote, velocitySend, MIDIchannel); //  send Note Off message 
        if (instrumentMode <2) {
          usbMIDI.sendNoteOff(activeNoteL, velocitySend, (MIDIchannel+1)); //  send Note Off message 
        }
        digitalWrite(13,LOW);
        breathLevel=0;
        state = NOTE_OFF;
      } else {
//        modeSelector();
        readSwitches();
        if (fingeredNote != lastFingering || fingeredNoteL !=lastFingeringL ){ //
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
            breathValHires = breathCurve(map(constrain(breathLevel,ON_Thr,breath_max),ON_Thr,breath_max,0,16383));
            velocitySend = (breathValHires >>7) & 0x007F;
            velocitySend = constrain(velocitySend,1,127); // set new velocity value based on current pressure sensor level
            usbMIDI.sendNoteOn(fingeredNote, velocitySend, MIDIchannel); // send Note On message for new note         
            usbMIDI.sendNoteOff(activeNote, 0, MIDIchannel); // send Note Off message for previous note (legato)
            activeNote=fingeredNote;
          }
          if (fingeredNoteL != activeNoteL) {
            // Player has moved to a new fingering while still blowing.
            // Send a note off for the current note and a note on for
            // the new note.      
            breathValHires = breathCurve(map(constrain(breathLevel,ON_Thr,breath_max),ON_Thr,breath_max,0,16383));
            velocitySend = (breathValHires >>7) & 0x007F;
            velocitySend = constrain(velocitySend,1,127); // set new velocity value based on current pressure sensor level
            usbMIDI.sendNoteOn(fingeredNoteL, velocitySend, (MIDIchannel+1)); // send Note On message for new note         
            usbMIDI.sendNoteOff(activeNoteL, 0, (MIDIchannel+1)); // send Note Off message for previous note (legato)
            activeNoteL=fingeredNoteL;
          }
        }
      }
    }
    // Is it time to send more CC data?
    if (millis() - ccSendTime > CC_INTERVAL) {
      // deal with Breath, Pitch Bend and Modulation
      breath();
      modulation();
      pitch_bend();
      ccSendTime = millis();
      readcc16();
    }
    lastFingering=fingeredNote; 
    lastFingeringL=fingeredNoteL; 
  }
}
//_______________________________________________________________________________________________ FUNCTIONS

// non linear mapping function (http://playground.arduino.cc/Main/MultiMap)
// note: the _in array should have increasing values
unsigned int multiMap(unsigned int val, unsigned int* _in, unsigned int* _out, uint8_t size)
{
  // take care the value is within range
  // val = constrain(val, _in[0], _in[size-1]);
  if (val <= _in[0]) return _out[0];
  if (val >= _in[size-1]) return _out[size-1];

  // search right interval
  uint8_t pos = 1;  // _in[0] allready tested
  while(val > _in[pos]) pos++;

  // this will handle all exact "points" in the _in array
  if (val == _in[pos]) return _out[pos];

  // interpolate in the right segment for the rest
  return (val - _in[pos-1]) * (_out[pos] - _out[pos-1]) / (_in[pos] - _in[pos-1]) + _out[pos-1];
}

//**************************************************************

// map breath values to selected curve
unsigned int breathCurve(unsigned int inputVal){
  // 0 to 16383, moving mid value up or down
  switch (curve){
    case 0:
      // -4
      return multiMap(inputVal,curveIn,curveM4,17);
      break;
    case 1:
      // -3
      return multiMap(inputVal,curveIn,curveM3,17);
      break;
    case 2:
      // -2
      return multiMap(inputVal,curveIn,curveM2,17);
      break;
    case 3:
      // -1
      return multiMap(inputVal,curveIn,curveM1,17);
      break;
    case 4:
      // 0, linear
      return inputVal;
      break;
    case 5:
      // +1
      return multiMap(inputVal,curveIn,curveP1,17);
      break;
    case 6:
      // +2
      return multiMap(inputVal,curveIn,curveP2,17);
      break;
    case 7:
      // +3
      return multiMap(inputVal,curveIn,curveP3,17);
      break;
    case 8:
      // +4
      return multiMap(inputVal,curveIn,curveP4,17);
      break;
    case 9:
      // S1
      return multiMap(inputVal,curveIn,curveS1,17);
      break;
    case 10:
      // S2
      return multiMap(inputVal,curveIn,curveS2,17);
      break;
    case 11:
      // S3
      return multiMap(inputVal,curveIn,curveS3,17);
      break;
    case 12:
      // S4
      return multiMap(inputVal,curveIn,curveS4,17);
      break;
  }
}

//**************************************************************

void breath(){
  int breathCCval,breathCCvalFine;
  unsigned int breathCCvalHires;
  breathLevel = constrain(pressureSensor,ON_Thr,breath_max);
  //breathCCval = map(constrain(breathLevel,ON_Thr,breath_max),ON_Thr,breath_max,0,127);
  breathCCvalHires = breathCurve(map(constrain(breathLevel,ON_Thr,breath_max),ON_Thr,breath_max,0,16383));
  breathCCval = (breathCCvalHires >>7) & 0x007F;
  breathCCvalFine = breathCCvalHires & 0x007F;
  //Serial.println(breathCCval);
  if (breathCCval != oldbreath){ // only send midi data if breath has changed from previous value
    if (breathCC){
      // send midi cc
      usbMIDI.sendControlChange(ccList[breathCC], breathCCval, MIDIchannel);
      if (instrumentMode <2){
        usbMIDI.sendControlChange(ccList[breathCC], breathCCval, (MIDIchannel+1));
      }
    }
    if (breathAT){
      // send aftertouch
      usbMIDI.sendAfterTouch(breathCCval, MIDIchannel);
      if (instrumentMode <2){
        usbMIDI.sendAfterTouch(breathCCval, (MIDIchannel+1));
      }
    }
    oldbreath = breathCCval;
  }
  if (breathCCvalHires != oldbreathhires){
    if (breathCC > 4){ // send high resolution midi
      usbMIDI.sendControlChange(ccList[breathCC]+32, breathCCvalFine, MIDIchannel);
    }
    oldbreathhires = breathCCvalHires;   
  }
  computePixels();
  pixels.show();   // Send the updated pixel colors to the hardware.
}

//**************************************************************
void pitch_bend(){
//  pitchBend = analogRead(A0); // read voltage on analog pin A0
  pitchBend = 0;
  if (pitchBend > modsHi_Thr){
    pitchBend = oldpb*0.6+0.4*map(constrain(pitchBend,modsHi_Thr,modsMax),modsHi_Thr,modsMax,0,8192); // go from 8192 to 16383 (full pb up) when off center threshold going up
  } 
  else if (pitchBend < modsLo_Thr){
    pitchBend = oldpb*0.6+0.4*map(constrain(pitchBend,modsMin,modsLo_Thr),modsMin,modsLo_Thr,-8192,0); // go from 8192 to 0 (full pb dn) when off center threshold going down
  } 
  else {
    pitchBend = oldpb - (oldpb *0.8); // released, so smooth your way back to zero
    if ((pitchBend > -100) && (pitchBend < 200)) {pitchBend = 0;}; // 8192 is 0 pitch bend, don't miss it bc of smoothing
  }
  if (mod || (ccList[breathCC]==1)){
    if (PB) pitchBend += lfoLevel/6; else pitchBend += lfoLevel;
    pitchBend=constrain(pitchBend, 0, 16383);
  }
  if (pitchBend != oldpb){// only send midi data if pitch bend has changed from previous value
    usbMIDI.sendPitchBend(pitchBend, MIDIchannel);
    oldpb=pitchBend;
    if (instrumentMode <2){
      usbMIDI.sendPitchBend(pitchBend, (MIDIchannel+1));
    }
  }
}


//***********************************************************
//
void modulation(){
//  int modRead = analogRead(A6); // read voltage on analog pin A6
  int modRead= 0;
  if (modRead < modsLo_Thr+2000){
    modLevel = map(constrain(modRead,modsMin,modsLo_Thr),modsMin,modsLo_Thr,127,0); // go from 0 to full modulation when off center threshold going left(?)
  } else {
    modLevel = 0; // zero modulation in center position
  }
  if (modLevel != oldmod){  // only send midi data if modulation has changed from previous value
    if (!mod  && (ccList[breathCC] != modCCnumber)) usbMIDI.sendControlChange(modCCnumber, modLevel, MIDIchannel);
    if (instrumentMode <2){
        usbMIDI.sendControlChange(modCCnumber, modLevel, (MIDIchannel+1));
       }
    oldmod=modLevel;
  }
  if (mod || (ccList[breathCC] == modCCnumber)) {
    lfo = waveformsTable[(millis()/2)%maxSamplesNum] - 2047;
    lfoLevel = lfo * modLevel / 1024 * lfoDepth;
  }
  if (portamento && (modRead > modsHi_Thr)) {    // if we are enabled and over the threshold, send portamento
   if (!portIsOn) {
      if (portamento == 2){ // if portamento midi switching is enabled
        usbMIDI.sendControlChange(CCN_PortOnOff, 127, MIDIchannel);
        if (instrumentMode <2){
        usbMIDI.sendControlChange(CCN_PortOnOff, 127, (MIDIchannel+1));
        }
      }
    portIsOn=1;
    }
    int portCC;
    portCC = map(constrain(modRead,modsHi_Thr,modsMax),modsHi_Thr,modsMax,0,portaMax); // go from 0 to full when off center threshold going right(?)
    if (portCC!=oldport){
      usbMIDI.sendControlChange(CCN_Port, portCC, MIDIchannel);
        if (instrumentMode <2){
          usbMIDI.sendControlChange(CCN_Port, portCC, (MIDIchannel+1));
        }
    }
    oldport = portCC;        
  } else if (portIsOn) {                                    // we have just gone below threshold, so send zero value
    usbMIDI.sendControlChange(CCN_Port, 0, MIDIchannel);
      if (instrumentMode <2){
        usbMIDI.sendControlChange(CCN_Port, 0, (MIDIchannel+1));
      }
    if (portamento == 2){                                   // if portamento midi switching is enabled
      usbMIDI.sendControlChange(CCN_PortOnOff, 0, MIDIchannel);
        if (instrumentMode <2){
          usbMIDI.sendControlChange(CCN_PortOnOff, 0, (MIDIchannel+1));
        }
    }
    portIsOn=0;
    oldport = 0; 
  }
}


//***********************************************************

void writeSetting(byte address, unsigned short value){
  union {
    byte v[2];
    unsigned short val;
  } data;
  data.val = value;
  EEPROM.write(address, data.v[0]);
  EEPROM.write(address+1, data.v[1]);  
}

//***********************************************************

unsigned short readSetting(byte address){
  union {
    byte v[2];
    unsigned short val;
  } data;  
  data.v[0] = EEPROM.read(address); 
  data.v[1] = EEPROM.read(address+1); 
  return data.val;
}

//***********************************************************

void readSwitches(){  
  pixels.clear();

  ////if you want to test if the #4 is touched, you can use
//if (cap.touched() & (1 << 4)) { do something }
////You can check its not touched with:
//if (! (cap.touched() & (1 << 4)) ) { do something }  

  // Read switches and put value in variables
  LH1=touchRead(17)>touch_Thr; //this will return true, or 1, if the touchRead measurement is greater than the touch_Thr, or false / 0 if it's lesser. Then, this 1 or 0 is used in the fingering equations below
  LH2=touchRead(0)>touch_Thr;
  LH3=touchRead(25)>touch_Thr;
  LH4=cap.touched() & (1 << 6); // this should be a true or false, or 1 or 0. The MPR121 board does the threshold stuff automatically, and just returns a 16 bit number when we use cap.touched(). The lower 12 bits of that number correspond to each of the 12 touchpads
  RH1=cap.touched() & (1 << 7);
  RH2=touchRead(22)>touch_Thr;
  RH3=touchRead(23)>touch_Thr;
  RH4=touchRead(1)>touch_Thr;
  RH5=cap.touched() & (1 << 4);
  RH6=cap.touched() & (1 << 5);
  TH1=cap.touched() & (1 << 0);
  TH2=cap.touched() & (1 << 1);
  TH3=cap.touched() & (1 << 2);
  TH4=cap.touched() & (1 << 3);
  OCTup=touchRead(15)>touch_Thr;
  OCTdn=touchRead(16)>touch_Thr;
  computePixels();
  modeSelector();
  pixels.show();   // Send the updated pixel colors to the hardware.
  
}
void modeSelector(){
  instrumentMode = instrumentModeCounter % numberOfModes;
        pixels.setPixelColor(instrumentMode, 20, 50, 0);
//  Serial.println(instrumentMode);
  //calculate midi note number from pressed keys  
  switch (instrumentMode){
    case 0:
      fingeredNote=startNote + (2*LH1 + LH2 + 3*LH3 + 7*LH4) - (2*RH1 + RH2 + 3*RH3 + 7*RH4 + 2*RH5 + RH6) + (7*TH1 + 5*TH2) - (5*TH3 + 7*TH4) + (12*OCTup) - (12*OCTdn) + (octave*12) + transposition; // Tuba mode
      break;
    case 1:
      fingeredNote=startNote - (2*LH1) - (LH2) - 4*LH3 - (7*LH4) + ((2*RH1) + (RH2) + 4*RH3 + (7*RH4) + 2*RH5 + RH6) + (7*TH1 + 5*TH2) + (7*TH1 + 5*TH2) - (5*TH3 + 7*TH4)  - (5*TH3 + 7*TH4) + (12*OCTup) - (12*OCTdn) + (octave*12) + transposition; // LH  is descending intervals, RH is ascending intervals
      break;
    case 2:
      fingeredNote=startNote + ((2*RH1) + (RH2) + 4*RH3 + (7*RH4) + 2*RH5 + RH6) + (7*TH1 + 5*TH2) - (5*TH3 + 7*TH4)  + (12*OCTup) - (12*OCTdn) + (octave*12) + transposition; // Right hand
      fingeredNoteL=startNote + ((2*LH1) + (LH2) + 4*LH3 + (7*LH4)) + (7*TH1 + 5*TH2) - (5*TH3 + 7*TH4)  + (12*OCTup) - (12*OCTdn) + (octave*12) + transposition -12; // Left hand
      break;
    case 3:
      fingeredNote=startNote + ((2*RH1) + (RH2) + 4*RH3 + (7*RH4) + 2*RH5 + RH6) + (7*TH1 + 5*TH2) - (5*TH3 + 7*TH4) + (12*OCTup) - (12*OCTdn) + (octave*12) + transposition; // Right hand
      fingeredNoteL=fingeredNote + ((2*LH1) + (LH2) + 4*LH3 + (7*LH4)) -12; // Left hand
      break;
    case 4:
      fingeredNote=startNote + ((2*RH1) + (RH2) + 4*RH3 + (7*RH4) + 2*RH5 + RH6) + (7*TH1 + 5*TH2) - (5*TH3 + 7*TH4) + (12*OCTup) - (12*OCTdn) + (octave*12) + transposition; // Right hand
      fingeredNoteL=fingeredNote + ((2*LH1) + (LH2) + 4*LH3 + (7*LH4)) -24; // Left hand
      break;
    case 5:
      fingeredNote=startNote + ((2*RH1) + (RH2) + 4*RH3 + (7*RH4) + 2*RH5 + RH6) + (7*TH1 + 5*TH2) - (5*TH3 + 7*TH4) + (12*OCTup) - (12*OCTdn) + (octave*12) + transposition; // Right hand
      fingeredNoteL=fingeredNote + ((2*LH1) + (LH2) + 4*LH3 + (7*LH4)); // Left hand
      break;
    case 6:
      fingeredNote=fingeredNote=startNote - (2*LH1) - (LH2) - 4*LH3 - (7*LH4) + ((2*RH1) + (RH2) + 4*RH3 + (7*RH4) + 2*RH5 + RH6) + (7*TH1 + 5*TH2) - (5*TH3 + 7*TH4) + (12*OCTup) - (12*OCTdn) + (octave*12) + transposition; // Option 1
      fingeredNoteL=startNote-24+transposition; // Left hand
      break;
  }
}

void computePixels(){
  pitchClass = fingeredNote % 12;
  int pixelNumber = pitchClass;
  for (int i = 0; i < 12; i = i + 1) {
    if(i != pitchClass || i !=instrumentMode) {
      pixelBrightnessMap[i] = 0;
      } 
    else {
      pixelBrightnessMap[pixelNumber] = velocitySend/2;
      }
    pixels.setPixelColor(pixelNumber, pixels.Color((velocitySend), 0, (65 - (velocitySend/2))));
    pixels.setPixelColor(instrumentMode, pixels.Color(30, 80, 0));
  }
  
  pixels.show();
  
}
//***********************************************************

void numberBlink(byte number){
  for (int i=0; i < number; i++){
    digitalWrite(13,HIGH);
    delay(200);
    digitalWrite(13,LOW);
    delay(200);
  }
  if (number == 0){
    digitalWrite(13,HIGH);
    delay(30);
    digitalWrite(13,LOW);
    delay(30);
    digitalWrite(13,HIGH);
    delay(30);
    digitalWrite(13,LOW);
    delay(30); 
    digitalWrite(13,HIGH);
    delay(30);
    digitalWrite(13,LOW);
    delay(30);   
    digitalWrite(13,HIGH);
    delay(30);
    digitalWrite(13,LOW);
    delay(200);
  }
}

//***********************************************************

void instrumentSettings(){  
    int y = 0;
    int x = 0;
//  int y = analogRead(A0); // read joystick y axis, A0
//  int x = analogRead(A6); // read joystick x axis, A6
  byte sel = 0;
  byte bin = 0;
  byte p1,p2;
  
  if      (y > ((modsHi_Thr+modsMax)/2)) sel = 1; // PB up -> Breath CC/AT settings
  else if (y < ((modsLo_Thr+modsMin)/2)) sel = 2; // PB dn -> MIDI CH setting
  else if (x > ((modsHi_Thr+modsMax)/2)) sel = 3; // Glide -> Various on/off
  else if (x < ((modsLo_Thr+modsMin)/2)) sel = 4; // Mod   -> Breath curve setting

  numberBlink(sel);
  
  delay(1000);
  
  if (sel){
    // Read switches and put value in variables
    bin = (touchRead(30)>touch_Thr) + (touchRead(22)>touch_Thr)*2 + (touchRead(23)>touch_Thr)*4;
    p1 = touchRead(1)>touch_Thr;
    p2 = touchRead(0)>touch_Thr;

    switch (sel){
      case 1:
        breathCC = constrain(bin,0,8);
        breathAT = p1;
        writeSetting(BREATH_CC_ADDR,breathCC);
        writeSetting(BREATH_AT_ADDR,breathAT);
        break;
      case 2:
        MIDIchannel = bin + p1*8 + 1;
        writeSetting(MIDI_ADDR,MIDI_FACTORY);
        break;
      case 3:
        if (bitRead(bin,0)){
          if (portamento) portamento = 0; else portamento = 2; // portamento 2 is both switch on/off and portamento value, portamento 1 is just value
          writeSetting(PORTAM_ADDR,portamento);
        }
        if (bitRead(bin,1)){
          PB = !PB;
          writeSetting(PB_ADDR,PB);
        }
        if (bitRead(bin,2)){
          mod = !mod;
          writeSetting(EXTRA_ADDR,mod);
        }
        if (p1){
          trans1 = !trans1;
          writeSetting(TRANS1_ADDR,trans1);
        }
        if (p2){
          trans2 = !trans2;
          writeSetting(TRANS2_ADDR,trans2);
        }
        if ((bin == 7) && p1 && p2){ //restore factory settings
          writeSetting(MIDI_ADDR,MIDI_FACTORY);
          writeSetting(TRANS1_ADDR,TRANS1_FACTORY);
          writeSetting(TRANS2_ADDR,TRANS2_FACTORY);
          writeSetting(BREATH_CC_ADDR,BREATH_CC_FACTORY);
          writeSetting(BREATH_AT_ADDR,BREATH_AT_FACTORY);
          writeSetting(PORTAM_ADDR,PORTAM_FACTORY);
          writeSetting(PB_ADDR,PB_FACTORY);
          writeSetting(EXTRA_ADDR,EXTRA_FACTORY);
          writeSetting(BREATHCURVE_ADDR,BREATHCURVE_FACTORY);
          MIDIchannel  = readSetting(MIDI_ADDR);
          trans1       = readSetting(TRANS1_ADDR);
          trans2       = readSetting(TRANS2_ADDR);
          //breathCC   = readSetting(BREATH_CC_ADDR);
          breathCC     = 0x02;
          breathAT     = readSetting(BREATH_AT_ADDR);
          portamento   = readSetting(PORTAM_ADDR);
          PB           = readSetting(PB_ADDR);
          mod          = readSetting(EXTRA_ADDR);
          curve        = readSetting(BREATHCURVE_ADDR);
        }
        break;
      case 4:
        curve = constrain((bin+p1*8),0,12);
        writeSetting(BREATHCURVE_ADDR,curve);
    }
    numberBlink(bin);
    delay(1000);
    numberBlink(p1);
    delay(1000);
    numberBlink(p2);
  }
}

void readcc16(){
  cc16pad=touchRead(32);
//  Serial.println(cc16pad);
  if (cc16pad > 3000){
    cc16pad=constrain(cc16pad, 3000, 4500);
    int factor =0.4;
    if (cc16pad < 4000) {
      factor =0.1;
    }
    cc16pad = oldcc16*0.6+0.4*map(cc16pad, 3000, 4500, 0, 127);
  } 
  else {
    cc16pad = 0;
  }
  if (cc16pad != oldcc16){// only send midi data if cc16 has changed from previous value
    usbMIDI.sendControlChange(16, cc16pad, MIDIchannel);
    oldcc16=cc16pad;
    
  }
}
