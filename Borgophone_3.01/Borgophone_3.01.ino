// I've brought most of the functions into this project from others. Currently there are no errors (missing variables, syntax errors, etc), but program is not complete. 
// Next step is to build the loop to call all the functions up. 
// next, set up breath in & out code.
// then, will need to write function for FSR. 
// then, create new LED drawing routine
// then, write code to brighten/dim frontLED
// would also be really cool to write a function to calibrate the touch sensors, like i'm doing for breath. 

#include <Bounce2.h>
#include "FastLED.h" //the FastLED library that drives all the LED data formatting. This project assumes you're using a strand of neoPixels
#include "Filters.h"  // for the breath signal LP filtering, https://github.com/edgar-bonet/Filters
#include <Wire.h>

// *************** pixel setup ****************
#define NUM_LEDS 12 // How many leds in your strip?
#define BRIGHTNESS  24
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 100

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;


CRGB leds[NUM_LEDS]; // Define the array of leds

//volatile byte state = LOW; //for interrupt

// ***************Pin definitions***************

// TOUCHPADS
#define touch_Thr 1300  // threshold for Teensy touchRead, 1300-1800
#define RH1pin 0
#define RH2pin 1
#define RH3pin 22
#define RH4pin 23
#define LH1pin 15
#define LH2pin 16
#define LH3pin 17
#define LH4pin 18
#define TH1pin 33
#define TH2pin 32
#define TH3pin 25
#define TH4pin 19

            // Key variables, TRUE (1) for pressed, FALSE (0) for not pressed
byte LH1;   
byte LH2;  
byte LH3;   
byte LH4;  
byte RH1;   
byte RH2;  
byte RH3;   
byte RH4;  
byte TH1;
byte TH2;
byte TH3;
byte TH4;
byte OCTup; // Octave switch key (pitch change +12) 
byte OCTdn; // Octave switch key (pitch change -12) 

// BUTTONS
#define octUppin 26 // button to transpose instrument an octave up
#define octDnpin 27 // button to transpose instrument an octave down
#define chUppin 4
#define chDnpin 5
#define progChUppin 6
#define progChDnpin 7
#define modeSelpin 11
#define curveSelpin 12

// BUTTON DEBOUNCING
int debounceInterval = 5;
bool OctUpState = 0;
bool OctDnState = 0;
bool ProgChgupState = 0;
bool ProgChgDnState = 0;
bool MidChUpState = 0;
bool MidChDnState = 0;
Bounce OctUpDebo = Bounce();// Instantiate a Bounce object
Bounce OctDnDebo = Bounce();
Bounce ProgChgupDebo = Bounce();
Bounce ProgChgDnDebo = Bounce();
Bounce MidChUpDebo = Bounce();
Bounce MidChDnDebo = Bounce();
Bounce ModeDebo = Bounce();
Bounce CurvePinDebo = Bounce();
Bounce OctADebo = Bounce();
Bounce OctBDebo = Bounce();

// SWITCHES
#define ccSwitchpin 24
#define OctApin 2 // limit switch for momentary octave up
#define OctBpin 3 // limit switch for momentary octave down

// LEDS
#define pixelPin 8
#define faceLEDpin 9
#define frontLEDpin 10
#define onboardLEDpin 13

// POTENTIOMETERS / ANALOG
#define joyXpin 21
#define joyYpin 20
#define pot1pin 28
#define pot2pin 30
#define pot3pin A11
#define pot4pin A10
#define breathPin A0 //A12
#define fsrpin A13
#define pot1CC 16
#define pot2CC 7
#define pot3CC 91
#define pot4CC 92
#define fsrCC 93
#define joyYCC 94
#define modCCnumber 1
int joyXReading = 0;
int joyYReading = 0;
int pot1Reading = 0;
int pot2Reading = 0;
int pot3Reading = 0;
int pot4Reading = 0;
int breathReading = 0;
int fsrReading = 0;

// BUTTON STATES
int octUpState = 0; 
int octDnState = 0; 
int chUpState = 0; 
int chDnState = 0; 
int progChUpState = 0; 
int progChDnState = 0;
int modeSelState = 0; 
int curveSelState = 0; 
int ccSwitchState = 0;
int OctAState = 0;
int OctBState = 0;

// DECLARATIONS
#define touch_Thr 1300  // threshold for Teensy touchRead, 1300-1800
#define ON_Thr 3500      // Set threshold level before switching ON
#define ON_Delay   7   // Set Delay after ON threshold before velocity is checked (wait for tounging peak)
#define breath_max 4095 // Threshold for maximum breath
int breath_min = 2900; // Threshold for minimum breath
#define modsLo_Thr 2600 // Low threshold for mod stick center
#define modsHi_Thr 3400 // High threshold for mod stick center
#define modsMin 1130     // PSP joystick min value
#define modsMax 4095    // PSP joystick max value
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
#define CC_INTERVAL 6
#define maxSamplesNum 120

// PROGRAM VARIABLES
bool calibrated = false;
int state;                         // The state of the state machine
unsigned long ccSendTime = 0L;     // The last time we sent CC values
unsigned long breath_on_time = 0L; // Time when breath sensor value went over the ON threshold
unsigned long pot1_change_time = 0L;
unsigned long pot2_change_time = 0L;
unsigned long pot3_change_time = 0L;
unsigned long pot4_change_time = 0L;
unsigned long fsr_change_time = 0L;
int initial_breath_value;          // The breath value at the time we observed the transition
unsigned long lastDebounceTime = 0;         // The last time the fingering was changed
unsigned long debounceDelay = 30;           // The debounce time; increase if the output flickers
int lastFingering = 0;             // Keep the last fingering value for debouncing
int lastFingeringL = 0;
byte MIDIchannel=1;                // MIDI channel 1
unsigned short breathCC = 2;           // OFF:MW:BR:VL:EX:MW+:BR+:VL+:EX+
unsigned short breathAT;
unsigned short portamento;         // switching on cc65? just cc5 enabled? SW:ON:OFF
unsigned short curve = 4;              // selected curve
unsigned short PB;
unsigned short mod;
unsigned short trans1;
unsigned short trans2;
byte ccList[5] = {0,1,2,7,11};  // OFF, Modulation, Breath, Volume, Expression
byte portIsOn=0;     // keep track and make sure we send CC with 0 value when off threshold
int oldport=0;
bool monoIsOn = 0;
int modLevel;
int oldmod=0;
int pitchBend;
int oldpb=8192;
int oldPot1=0;
int oldPot2=0;
int oldPot3=0;
int oldPot4=0;
int oldfsr=0;
int potDeadZone=3;
int PB_sens;
int octave = 0; //MD
int ProgChangeNumber = 0; //MD
int fingeredNote;    // note calculated from fingering (switches) and octave joystick position
int fingeredNoteL;
byte activeNote;     // note playing
byte activeNoteL;
byte startNote = 70; //MD
float filterFreq = 60.0; 
unsigned int curveM4[] = {0,4300,7000,8700,9900,10950,11900,12600,13300,13900,14500,15000,15450,15700,16000,16250,16383};
unsigned int curveM3[] = {0,2900,5100,6650,8200,9500,10550,11500,12300,13100,13800,14450,14950,15350,15750,16150,16383};
unsigned int curveM2[] = {0,2000,3600,5000,6450,7850,9000,10100,11100,12100,12900,13700,14400,14950,15500,16000,16383};
unsigned int curveM1[] = {0,1400,2850,4100,5300,6450,7600,8700,9800,10750,11650,12600,13350,14150,14950,15650,16838};
unsigned int curveIn[] = {0,1023,2047,3071,4095,5119,6143,7167,8191,9215,10239,11263,12287,13311,14335,15359,16383};
unsigned int curveP1[] = {0,600,1350,2150,2900,3800,4700,5600,6650,7700,8800,9900,11100,12300,13500,14850,16383};
unsigned int curveP2[] = {0,400,800,1300,2000,2650,3500,4300,5300,6250,7400,8500,9600,11050,12400,14100,16383};
unsigned int curveP3[] = {0,200,500,900,1300,1800,2350,3100,3800,4600,5550,6550,8000,9500,11250,13400,16383};
unsigned int curveP4[] = {0,100,200,400,700,1050,1500,1950,2550,3200,4000,4900,6050,7500,9300,12100,16282};
unsigned int curveS1[] = {0,600,1350,2150,2900,3800,4700,6000,8700,11000,12400,13400,14300,14950,15500,16000,16383};
unsigned int curveS2[] = {0,600,1350,2150,2900,4000,6100,9000,11000,12100,12900,13700,14400,14950,15500,16000,16383};
unsigned int curveS3[] = {0,600,1350,2300,3800,6200,8700,10200,11100,12100,12900,13700,14400,14950,15500,16000,16383};
unsigned int curveS4[] = {0,600,1700,4000,6600,8550,9700,10550,11400,12200,12900,13700,14400,14950,15500,16000,16383};
int pitchClass = 0;
int pixelBrightnessMap[12] = {0,0,0,0,0,0,0,0,0,0,0,0}; // this is an array of 12 integers for the pixel's Brightness (HSB)
int sensorMidpoint = 820;
int sensorNeedle = sensorMidpoint;
int breathCalibrationMeasurement = 820;
int breathCalibrationTotal = 0;
int breathDeadZone = 80;
int breathLevel=0;   // breath level (smoothed) not mapped to CC value
int oldbreath=0;
unsigned int oldbreathhires=0;
unsigned int breathValHires=0;
int pressureSensor;  // pressure data from breath sensor, for midi breath cc and breath threshold checks
byte velocitySend;   // remapped midi velocity from breath sensor
unsigned long instrumentModeCounter = 0;
unsigned long curveCounter = 4;
int numberOfCurves = 13;
unsigned int numberOfModes = 7;
int instrumentMode = 0;

void setup() {
  // ***************Pin setups***************
  
  pinMode(octUppin, INPUT_PULLUP);
  pinMode(octDnpin, INPUT_PULLUP); 
  pinMode(chUppin, INPUT_PULLUP); 
  pinMode(chDnpin, INPUT_PULLUP); 
  pinMode(progChUppin, INPUT_PULLUP); 
  pinMode(progChDnpin, INPUT_PULLUP); 
  pinMode(modeSelpin, INPUT_PULLUP); 
  pinMode(curveSelpin, INPUT_PULLUP);
  pinMode(ccSwitchpin, INPUT_PULLUP);
  pinMode(OctApin, INPUT_PULLUP);
  pinMode(OctBpin, INPUT_PULLUP);
  pinMode(faceLEDpin, OUTPUT); 
  pinMode(frontLEDpin, OUTPUT);  
  pinMode(onboardLEDpin, OUTPUT); 
  Serial.begin(115200); // Go serial go!
  analogReadResolution(12);   // set resolution of ADCs to 12 bit
  state = NOTE_OFF;
  // *************** pixel setup ***************
  delay( 1000 ); // power-up safety delay
//  FastLED.addLeds<LED_TYPE, pixelPin, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
//  FastLED.setBrightness(  BRIGHTNESS );

  // *************** button debounce setups ***************
  OctUpDebo.attach(octUppin); // After setting up the button, setup the Bounce instance
  OctUpDebo.interval(debounceInterval); // interval in ms
  OctDnDebo.attach(octDnpin);
  OctDnDebo.interval(debounceInterval); // interval in ms
  ProgChgupDebo.attach(progChUppin);
  ProgChgupDebo.interval(debounceInterval); // interval in ms
  ProgChgDnDebo.attach(progChDnpin);
  ProgChgDnDebo.interval(debounceInterval); // interval in ms
  MidChUpDebo.attach(chUppin);
  MidChUpDebo.interval(debounceInterval); // interval in ms
  MidChDnDebo.attach(chDnpin);
  MidChDnDebo.interval(debounceInterval); // interval in ms
  ModeDebo.attach(modeSelpin);
  ModeDebo.interval(debounceInterval); // interval in ms
  CurvePinDebo.attach(curveSelpin);
  CurvePinDebo.interval(debounceInterval); // interval in ms
  OctADebo.attach(OctApin);
  OctADebo.interval(debounceInterval);
  OctBDebo.attach(OctBpin);
  OctBDebo.interval(debounceInterval);
}

// ***************************************************
void loop(){
  if(calibrated == false){
    Serial.println("calibrating!");
    calibrateBreath();
  }
  if (!monoIsOn){
    usbMIDI.sendControlChange(126, 2, MIDIchannel);
    usbMIDI.sendControlChange(126, 2, (MIDIchannel +1));
    monoIsOn = true;
  }
    // MIDI Controllers should discard incoming MIDI messages.
  while (usbMIDI.read()) {
  }
  mainLoop();
}
// ****************************************************

void calibrateBreath(){
  delay(400);
    for (int i = 0; i <= 10; i++) {
    int breathCalibrationMeasurement = analogRead(breathPin);
    Serial.println(breathCalibrationMeasurement);
    breathCalibrationTotal += breathCalibrationMeasurement;  
    if (i % 2 == 0){
      analogWrite(faceLEDpin, 255);
      analogWrite(frontLEDpin, 0);
    } else if (i % 2 == 1){
      analogWrite(faceLEDpin, 0);
      analogWrite(frontLEDpin, 255);
    }
    delay(20);
    if (i == 10) {
      sensorMidpoint = breathCalibrationTotal / 11;
      calibrated = true;
        Serial.println("Calibrated!!!");
        Serial.println(sensorMidpoint);
        analogWrite(faceLEDpin, 0);
        analogWrite(frontLEDpin, 0);
    }
  }
}

void mainLoop() {  
  while(1){
    readButtons();
    pressureSensor = measureBreath();
//    delay(250);
    if (state == NOTE_OFF) {
        if (abs(sensorNeedle) > breathDeadZone){ // if the needle has moved outside the deadzone (either upwards or downwards), let's get ready to send some midi data
        // Value has risen above threshold. Move to the ON_Delay
//        // state. Record time and initial breath value.
        breath_on_time = millis();
        pot1_change_time = millis();
        pot2_change_time = millis();
        pot3_change_time = millis();
        pot4_change_time = millis();
        fsr_change_time = millis();
        initial_breath_value = pressureSensor;
        state = RISE_WAIT;  // Go to next state
        readTouchPads();  
      }      
    } else if (state == RISE_WAIT) {
      if ((abs(sensorNeedle) > breathDeadZone)){
        // Has enough time passed for us to collect our second
        // sample?
        if (millis() - breath_on_time > ON_Delay ) {
          // Yes, so calculate MIDI note and velocity, then send a note on event
          readTouchPads();
          // We should be at tonguing peak, so set velocity based on current pressureSensor value        
          // If initial value is greater than value after delay, go with initial value, constrain input to keep mapped output within 1 to 127
          digitalWrite(faceLEDpin, HIGH);
          velocitySend = measureBreath();
          usbMIDI.sendNoteOn(fingeredNote, velocitySend, MIDIchannel); // send Note On message for new note 
          Serial.println("NOTE_ON");
          Serial.println(fingeredNote);
          if (instrumentMode >0){
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
        if ((abs(sensorNeedle) < breathDeadZone)) {
        // Value has fallen below threshold - turn the note off
        digitalWrite(faceLEDpin, LOW);
        usbMIDI.sendNoteOff(activeNote, 0, MIDIchannel); //  send Note Off message 
        usbMIDI.sendNoteOff(activeNoteL, 0, MIDIchannel + 1); //new
        Serial.println("NOTE_OFF");
        digitalWrite(13,LOW); 
        breathLevel=0;
        state = NOTE_OFF;
      } else {
        modeSelector();
        readTouchPads();
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
            velocitySend = measureBreath(); // set new velocity value based on current pressure sensor level
            usbMIDI.sendNoteOn(fingeredNote, velocitySend, MIDIchannel); // send Note On message for new note    
            usbMIDI.sendNoteOff(activeNote, 0, MIDIchannel); // send Note Off message for previous note (legato)
            activeNote=fingeredNote;
          }
          if (fingeredNoteL != activeNoteL) {
            velocitySend = measureBreath();
            Serial.println("newNote");
            usbMIDI.sendNoteOn(fingeredNoteL, velocitySend, (MIDIchannel+1)); // send Note On message for new note         
            usbMIDI.sendNoteOff(activeNoteL, 0, (MIDIchannel+1)); // send Note Off message for previous note (legato)
            activeNoteL=fingeredNoteL;
          }
        }
      }
    }
    // Is it time to send more CC data?
//      if (millis() - ccSendTime > CC_INTERVAL && (abs(pressureSensor - initial_breath_value) > potDeadZone)) {
      if (millis() - ccSendTime > CC_INTERVAL) {
      // deal with Breath, Pitch Bend and Modulation
      getBreathCC();
      modulation();
      pitch_bend();
      ccSendTime = millis();
      readPots();
//      Serial.println("time for CC");
//      readFSR();
    }
    lastFingering=fingeredNote; 
    lastFingeringL=fingeredNoteL; 
  }
}

int measureBreath(){
  bool breathCCselection = digitalRead(ccSwitchpin);
  if(breathCCselection == HIGH) {
    breathCC = 2; //0x02 is standard breath control CC
  } 
  else {
    breathCC = 11; //0x0B is good for Session Horns velocity
  }
  int breathMeasurement = analogRead(breathPin);
  
  breathLevel = constrain (breathMeasurement, 0, 4095);
  sensorNeedle = breathMeasurement - sensorMidpoint;
//  Serial.print("breathMeasurement ");
//  Serial.println(breathMeasurement);
    if (sensorNeedle <= -1 * (breathDeadZone / 2)){ // we're blowing
    int sensorNeedleLowBoundary = sensorMidpoint - breathDeadZone;
//    Serial.println("+");
//    Serial.print("sensorNeedleLowBoundary ");
//    Serial.println(sensorNeedleLowBoundary);
//    Serial.print("breathLevel ");
//    Serial.println(breathLevel);
    breathLevel=constrain(breathLevel, breath_min, sensorNeedleLowBoundary);
    breathLevel=map(breathLevel, sensorNeedleLowBoundary, breath_min, 0, 127);  
//    Serial.print("breath_min ");
//    Serial.println(breath_min);
    breathLevel=constrain(breathLevel,0,127);
    Serial.print("breathLevelReturn ");
    Serial.println(breathLevel);  
    return breathLevel;
  }
  if (sensorNeedle > breathDeadZone / 2) { // we're sucking
    int sensorNeedleLowBoundary = sensorMidpoint + breathDeadZone;
//    Serial.println("-");
    breathLevel=constrain(breathLevel, sensorNeedleLowBoundary, breath_max);
    breathLevel=map(breathLevel, sensorNeedleLowBoundary, breath_max, 0, 127);
    breathLevel=constrain(breathLevel,0,127);
    Serial.print("breathLevelReturn ");
    Serial.println(breathLevel);  
    return breathLevel;
  }
  else return 0;
}

void getBreathCC(){
  int breathCCval = measureBreath();
  breathCCval = constrain(breathCCval, 0, 127);
  if (breathCCval - oldbreath){ // only send midi data if breath has changed from previous value
    // send midi cc
    Serial.print("breathCCval ");
    Serial.println(breathCCval);
    usbMIDI.sendControlChange(ccList[breathCC], breathCCval, MIDIchannel);
    if (instrumentMode >1){
      usbMIDI.sendControlChange(ccList[breathCC], breathCCval, (MIDIchannel+1));
    }
    oldbreath = breathCCval;
  }
  
//  computePixels();
//  pixels.show();   // Send the updated pixel colors to the hardware.
}



void readTouchPads(){
  RH1 = touchRead(RH1pin)>touch_Thr;
  RH2 = touchRead(RH2pin)>touch_Thr;
  RH3= touchRead(RH3pin)>touch_Thr;
  RH4= touchRead(RH4pin)>touch_Thr;
  LH1= touchRead(LH1pin)>touch_Thr;
  LH2= touchRead(LH2pin)>touch_Thr;
  LH3= touchRead(LH3pin)>touch_Thr;
  LH4= touchRead(LH4pin)>touch_Thr;
  TH1= touchRead(TH1pin)>touch_Thr;
  TH2= touchRead(TH2pin)>touch_Thr;
  TH3= touchRead(TH3pin)>touch_Thr;
  TH4= touchRead(TH4pin)>touch_Thr;
  modeSelector();
}

void readButtons(){
  // Update the Bounce instances :
  OctUpDebo.update();
  OctDnDebo.update();
  ProgChgupDebo.update();
  ProgChgDnDebo.update();
  MidChUpDebo.update();
  MidChDnDebo.update();
  ModeDebo.update();
  CurvePinDebo.update();
  OctADebo.update();
  OctBDebo.update();

  if (OctUpDebo.fell()){
    octave ++;  // octave up
    Serial.println("Transposed Octave Up");
    Serial.println(octave);
  }
  if (OctDnDebo.fell()){
    octave --;  // octave down
    Serial.println("Transposed Octave Down");
    Serial.println(octave);
  }
  if (ProgChgupDebo.fell() && ProgChangeNumber < 127 ){
    ProgChangeNumber ++;  // transpose up half step
    Serial.println("Program Change");
    Serial.println(ProgChangeNumber);
    usbMIDI.sendProgramChange(ProgChangeNumber, MIDIchannel);
  }
  if (ProgChgDnDebo.fell() && ProgChangeNumber > 0 ){
    ProgChangeNumber --;  // transpose down half step
    Serial.println("Program Change");
    Serial.println(ProgChangeNumber);
    usbMIDI.sendProgramChange(ProgChangeNumber, MIDIchannel);
   }
  if (MidChUpDebo.fell()&& MIDIchannel < 16 ){
    MIDIchannel ++;
    Serial.println("Midi +");
    Serial.println(MIDIchannel);
    
  }
  if (MidChDnDebo.fell()&& MIDIchannel > 1 ){ //using Midi channel down for mode selection at the moment
    MIDIchannel --;
    Serial.println("Midi -");
    Serial.println(MIDIchannel);

  }
  if (ModeDebo.fell() ){
    if (instrumentModeCounter < numberOfModes){
      instrumentModeCounter++;
    } else {
      instrumentModeCounter = 0;
    }
    modeSelector();
    Serial.println("Mode");
    Serial.println(instrumentModeCounter);

  }
  if (CurvePinDebo.fell() ){
    curveCounter++;
    curveSelector();
    Serial.println("curve");
    Serial.println(curve);

  }
  if (OctADebo.fell() ){
    OCTup = 1;
    Serial.println("8va");
  }
  if (OctBDebo.fell() ){
    OCTdn = 1;
    Serial.println("8vb");
  }
  if (OctADebo.rose() ){
    OCTup = 0;
  }
  if (OctBDebo.rose() ){
    OCTdn = 0;
  }
}

void curveSelector(){
  curve = curveCounter % numberOfCurves;
}

void modeSelector(){
  instrumentMode = instrumentModeCounter % numberOfModes;
//  Serial.println(instrumentMode);
  //calculate midi note number from pressed keys  
  switch (instrumentMode){
    case 0:// Tuba mode
      fingeredNote=constrain(startNote + (2*LH1 + LH2 + 4*LH3 + 7*LH4) - (2*RH1 + RH2 + 3*RH3 + 5*RH4) - (5*TH1 + 7*TH2) + (7*TH3 + 5*TH4) + (12*OCTup) - (12*OCTdn) + (octave*12) 
      ,0, 127);
      break;
    case 1: //Duet RH +LH fingers
      fingeredNote=constrain(startNote - ((2*RH1) + (RH2) + 3*RH3 + (5*RH4)) - (5*TH1 + 7*TH2) + (7*TH3 + 5*TH4) + (12*OCTup) - (12*OCTdn) + (octave*12) // Right hand
      ,0, 127);
      fingeredNoteL=constrain(fingeredNote + ((2*LH1) + (LH2) + 3*LH3 + (7*LH4)) // Left hand
      ,0, 127);
      break;
    case 2: //Duet RH +LH fingers - 1 oct
      fingeredNote=constrain(startNote - ((2*RH1) + (RH2) + 3*RH3 + (5*RH4)) - (5*TH1 + 7*TH2) + (7*TH3 + 5*TH4) + (12*OCTup) - (12*OCTdn) + (octave*12) // Right hand
      ,0, 127);
      fingeredNoteL=constrain(fingeredNote + ((2*LH1) + (LH2) + 4*LH3 + (7*LH4)) -12 // Left hand
      ,0, 127);
      break;

    case 3: // Close harmony, easy thirds 
      fingeredNote=constrain(startNote - ((2*RH1) + (RH2) + 3*RH3 + (5*RH4)) - (5*TH1 + 7*TH2) + (7*TH3 + 5*TH4) + (12*OCTup) - (12*OCTdn) + (octave*12) // Right hand
      ,0, 127);
      fingeredNoteL=constrain(fingeredNote + ((4*LH1) + (3*LH2) + 2*LH3 + (5*LH4)) // Left hand
      ,0, 127);
      break;
    case 4: //TWO INDEPENDENT VOICES
      fingeredNote=constrain(startNote - ((2*RH1) + (RH2) + 4*RH3 + (5*RH4)) - (5*TH1 + 7*TH2) + (7*TH3 + 5*TH4)  + (12*OCTup) - (12*OCTdn) + (octave*12) //  Right hand
      ,0, 127);
      fingeredNoteL=constrain(startNote - ((2*LH1) + (LH2) + 4*LH3 + (5*LH4)) + (12*OCTup) - (12*OCTdn) + (octave*12) // Left hand
//      fingeredNoteL=constrain(startNote - ((2*LH1) + (LH2) + 4*LH3 + (5*LH4)) - (5*TH1 + 7*TH2) + (7*TH3 + 5*TH4)  + (12*OCTup) - (12*OCTdn) + (octave*12) // Left hand
      ,0, 127);
      break;

    case 5://Duet RH +LH fingers -2 oct
      fingeredNote=constrain(startNote - ((2*RH1) + (RH2) + 3*RH3 + (5*RH4)) - (5*TH1 + 7*TH2) + (7*TH3 + 5*TH4) + (12*OCTup) - (12*OCTdn) + (octave*12) // Right hand
      ,0, 127);
      fingeredNoteL=constrain(fingeredNote + ((2*LH1) + (LH2) + 4*LH3 + (7*LH4)) -24 // Left hand
      ,0, 127);
      break;

    case 6: // B flat drone
      fingeredNote=constrain(fingeredNote=startNote + (2*LH1) + (LH2) + 3*LH3 + (7*LH4) - ((2*RH1) + (RH2) + 3*RH3 + (5*RH4)) - (5*TH1 + 7*TH2) + (7*TH3 + 5*TH4) + (12*OCTup) - (12*OCTdn) + (octave*12) // Both hands
      ,0, 127);
      fingeredNoteL=constrain(startNote-12 // Drone note
      ,0, 127);
      break;

//    case 7: // One voice, lefty mode LH  is descending intervals, RH is ascending intervals
//      fingeredNote=constrain(startNote - (2*LH1) - (LH2) - 4*LH3 - (7*LH4) + ((2*RH1) + (RH2) + 4*RH3 + (7*RH4)) - (5*TH1 + 7*TH2) + (7*TH3 + 5*TH4) - (5*TH3 + 7*TH4)  - (5*TH3 + 7*TH4) + (12*OCTup) - (12*OCTdn) + (octave*12) 
//      ,0, 127);
//      break;
  }
}

void pitch_bend(){
  pitchBend = analogRead(joyXpin); // read voltage on analog pin A0
//  pitchBend = 0;
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
    pitchBend=constrain(pitchBend, 0, 16383);
  }
  if (pitchBend != oldpb){// only send midi data if pitch bend has changed from previous value
    usbMIDI.sendPitchBend(pitchBend, MIDIchannel);
    oldpb=pitchBend;
    if (instrumentMode >1){
      usbMIDI.sendPitchBend(pitchBend, (MIDIchannel+1));
    }
  }
}

void modulation(){
  int modRead = analogRead(joyYpin); // read voltage on analog pin A6
//  int modRead= 0;
  if (modRead < modsLo_Thr+2000){
    modLevel = map(constrain(modRead,modsMin,modsLo_Thr),modsMin,modsLo_Thr,127,0); // go from 0 to full modulation when off center threshold going left(?)
  } else {
    modLevel = 0; // zero modulation in center position
  }
  if (modLevel != oldmod){  // only send midi data if modulation has changed from previous value
    if (!mod  && (ccList[breathCC] != modCCnumber)) usbMIDI.sendControlChange(modCCnumber, modLevel, MIDIchannel);
    if (instrumentMode >1){
        usbMIDI.sendControlChange(modCCnumber, modLevel, (MIDIchannel+1));
       }
    oldmod=modLevel;
  }
//  if (mod || (ccList[breathCC] == modCCnumber)) {
//    lfo = waveformsTable[(millis()/2)%maxSamplesNum] - 2047;
//    lfoLevel = lfo * modLevel / 1024 * lfoDepth;
//  }
  if (portamento && (modRead > modsHi_Thr)) {    // if we are enabled and over the threshold, send portamento
   if (!portIsOn) {
      if (portamento == 2){ // if portamento midi switching is enabled
        usbMIDI.sendControlChange(CCN_PortOnOff, 127, MIDIchannel);
        if (instrumentMode >1){
        usbMIDI.sendControlChange(CCN_PortOnOff, 127, (MIDIchannel+1));
        }
      }
    portIsOn=1;
    }
    int portCC;
    portCC = map(constrain(modRead,modsHi_Thr,modsMax),modsHi_Thr,modsMax,0,portaMax); // go from 0 to full when off center threshold going right(?)
    if (portCC!=oldport){
      usbMIDI.sendControlChange(CCN_Port, portCC, MIDIchannel);
        if (instrumentMode >1){
          usbMIDI.sendControlChange(CCN_Port, portCC, (MIDIchannel+1));
        }
    }
    oldport = portCC;        
  } else if (portIsOn) {                                    // we have just gone below threshold, so send zero value
    usbMIDI.sendControlChange(CCN_Port, 0, MIDIchannel);
      if (instrumentMode >1){
        usbMIDI.sendControlChange(CCN_Port, 0, (MIDIchannel+1));
      }
    if (portamento == 2){                                   // if portamento midi switching is enabled
      usbMIDI.sendControlChange(CCN_PortOnOff, 0, MIDIchannel);
        if (instrumentMode >1){
          usbMIDI.sendControlChange(CCN_PortOnOff, 0, (MIDIchannel+1));
        }
    }
    portIsOn=0;
    oldport = 0; 
  }
}

void readPots(){
  pot1Reading = analogRead(pot1pin);
  pot2Reading = analogRead(pot2pin);
  pot3Reading = analogRead(pot3pin);
  pot4Reading = analogRead(pot4pin);
  int pot1Value=map(pot1Reading, 5, 4095, 0, 127 );
  if (abs(pot1Value - oldPot1) > potDeadZone){
    if (millis() - pot1_change_time > ON_Delay) {
    usbMIDI.sendControlChange(pot1CC, pot1Value, MIDIchannel);
    oldPot1=pot1Value;
    Serial.println(pot1Value);
    }
  }
  int pot2Value=map(pot2Reading, 5, 4095, 0, 127 );
  if (abs(pot2Value - oldPot2) > potDeadZone){
    if (millis() - pot2_change_time > ON_Delay) {
    usbMIDI.sendControlChange(pot2CC, pot2Value, MIDIchannel);
    oldPot2=pot2Value;
    Serial.println(pot2Value);
    }
  }
  int pot3Value=map(pot3Reading, 5, 4095, 0, 127 );
  if (abs(pot3Value - oldPot3) > potDeadZone){
    Serial.println(pot3Value);
    if (millis() - pot3_change_time > ON_Delay) {
    usbMIDI.sendControlChange(pot3CC, pot3Value, MIDIchannel);
    oldPot3=pot3Value;
    
    }
  }
  breath_min = constrain(pot4Reading, 0, sensorMidpoint - 400);
//  Serial.print("breath_min ");
//  Serial.println(breath_min);
//  int pot4Value=map(pot4Reading, 5, 4095, 0, 127 );
//  if (abs(pot4Value - oldPot4) > potDeadZone){
//    if (millis() - pot4_change_time > ON_Delay) {
//    usbMIDI.sendControlChange(pot4CC, pot4Value, MIDIchannel);
//    oldPot4=pot4Value;
//    }

//  }
}

void readFSR(){
  fsrReading = analogRead(fsrpin);
  Serial.println(fsrReading);
  if (fsrReading > 3000){
    int fsrValue=map(fsrReading, 3000, 4095, 0, 127 );
    if (abs(fsrValue - oldfsr) > potDeadZone) {
      if (millis() - fsr_change_time > ON_Delay) {
        Serial.println(fsrValue);
        usbMIDI.sendControlChange(fsrCC, fsrValue, MIDIchannel);
        oldfsr=fsrValue;
        if (fsrValue > 15) {
        usbMIDI.sendControlChange(fsrCC, 0, MIDIchannel);
        }
      }
    }
  }
}

// FASTLED animations

void computePixels(){
//  pitchClass = fingeredNote % 12;
//  int pixelNumber = pitchClass;
//  for (int i = 0; i < 12; i = i + 1) {
//    if(i != pitchClass || i !=instrumentMode) { // 
//      pixelBrightnessMap[i] = 0;
//      } 
//    else {
//      pixelBrightnessMap[pixelNumber] = velocitySend/2;
//      }
////    pixels.setPixelColor(pixelNumber, pixels.Color((velocitySend), 0, (65 - (velocitySend/2))));
//    //pixels.setPixelColor(instrumentMode, pixels.Color(30, 80, 0));
//  }
//  
////  pixels.show();
//  
}
//
//void FastLEDDisplay(){
//static uint8_t startIndex = 0;
//  startIndex = startIndex + 1; /* motion speed */
//  FillLEDsFromPaletteColors( startIndex);
//  ChangePalettePeriodically();
//  FastLED.show();
//  FastLED.delay(1000 / UPDATES_PER_SECOND);
//}
//void FillLEDsFromPaletteColors( uint8_t colorIndex)
//{
//    uint8_t brightness = 255;
//    
//    for( int i = 0; i < NUM_LEDS; i++) {
//        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
//        colorIndex += 3;
//    }
//}
//void ChangePalettePeriodically(){
//  uint8_t secondHand = (millis() / 1000) % 60;
//  static uint8_t lastSecond = 99;
//  
//  if( lastSecond != secondHand) {
//    lastSecond = secondHand;
//    if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
//    if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
//    if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
//    if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
//    if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
//    if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
//    if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
//    if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
//    if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
//    if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
//    if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
//  }
//}
//void SetupTotallyRandomPalette(){
//  for( int i = 0; i < 16; i++) {
//    currentPalette[i] = CHSV( random8(), 255, random8());
//  }
//}
//void SetupBlackAndWhiteStripedPalette()
//{
//    // 'black out' all 16 palette entries...
//    fill_solid( currentPalette, 16, CRGB::Black);
//    // and set every fourth one to white.
//    currentPalette[0] = CRGB::White;
//    currentPalette[4] = CRGB::White;
//    currentPalette[8] = CRGB::White;
//    currentPalette[12] = CRGB::White;
//}
//// This function sets up a palette of purple and green stripes.
//void SetupPurpleAndGreenPalette()
//{
//    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
//    CRGB green  = CHSV( HUE_GREEN, 255, 255);
//    CRGB black  = CRGB::Black;
//    
//    currentPalette = CRGBPalette16(
//                                   green,  green,  black,  black,
//                                   purple, purple, black,  black,
//                                   green,  green,  black,  black,
//                                   purple, purple, black,  black );
//}
//// This example shows how to set up a static color palette
//// which is stored in PROGMEM (flash), which is almost always more
//// plentiful than RAM.  A static PROGMEM palette like this
//// takes up 64 bytes of flash.
//const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
//{
//    CRGB::Red,
//    CRGB::Gray, // 'white' is too bright compared to red and blue
//    CRGB::Blue,
//    CRGB::Black,
//    
//    CRGB::Red,
//    CRGB::Gray,
//    CRGB::Blue,
//    CRGB::Black,
//    
//    CRGB::Red,
//    CRGB::Red,
//    CRGB::Gray,
//    CRGB::Gray,
//    CRGB::Blue,
//    CRGB::Blue,
//    CRGB::Black,
//    CRGB::Black
//};
//
//// non linear mapping function (http://playground.arduino.cc/Main/MultiMap)
//// note: the _in array should have increasing values
//unsigned int multiMap(unsigned int val, unsigned int* _in, unsigned int* _out, uint8_t size){
//  // take care the value is within range
//  // val = constrain(val, _in[0], _in[size-1]);
//  if (val <= _in[0]) return _out[0];
//  if (val >= _in[size-1]) return _out[size-1];
//
//  // search right interval
//  uint8_t pos = 1;  // _in[0] allready tested
//  while(val > _in[pos]) pos++;
//
//  // this will handle all exact "points" in the _in array
//  if (val == _in[pos]) return _out[pos];
//
//  // interpolate in the right segment for the rest
//  return (val - _in[pos-1]) * (_out[pos] - _out[pos-1]) / (_in[pos] - _in[pos-1]) + _out[pos-1];
//}
//
//// map breath values to selected curve
//unsigned int breathCurve(unsigned int inputVal){
//  // 0 to 16383, moving mid value up or down
//  switch (curve){
//    case 0:
//      // -4
//      return multiMap(inputVal,curveIn,curveM4,17);
//      break;
//    case 1:
//      // -3
//      return multiMap(inputVal,curveIn,curveM3,17);
//      break;
//    case 2:
//      // -2
//      return multiMap(inputVal,curveIn,curveM2,17);
//      break;
//    case 3:
//      // -1
//      return multiMap(inputVal,curveIn,curveM1,17);
//      break;
//    case 4:
//      // 0, linear
//      return inputVal;
//      break;
//    case 5:
//      // +1
//      return multiMap(inputVal,curveIn,curveP1,17);
//      break;
//    case 6:
//      // +2
//      return multiMap(inputVal,curveIn,curveP2,17);
//      break;
//    case 7:
//      // +3
//      return multiMap(inputVal,curveIn,curveP3,17);
//      break;
//    case 8:
//      // +4
//      return multiMap(inputVal,curveIn,curveP4,17);
//      break;
//    case 9:
//      // S1
//      return multiMap(inputVal,curveIn,curveS1,17);
//      break;
//    case 10:
//      // S2
//      return multiMap(inputVal,curveIn,curveS2,17);
//      break;
//    case 11:
//      // S3
//      return multiMap(inputVal,curveIn,curveS3,17);
//      break;
//    case 12:
//      // S4
//      return multiMap(inputVal,curveIn,curveS4,17);
//      break;
//  }
//}
