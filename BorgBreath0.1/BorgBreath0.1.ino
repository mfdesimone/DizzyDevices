/* USB MIDI AnalogControlChange Example

   You must select MIDI from the "Tools > USB Type" menu
   http://www.pjrc.com/teensy/td_midi.html

   This example code is in the public domain.
*/

//#include <Bounce.h>


// the MIDI continuous controller for each analog input
int blowCC = 2; // Breath CC
int suckCC = 11;
const int breathPin = 14;
const int sensitivityPin = 15;
const int ccSwitchPin = 11;
const int ledBlowPin = 10;
const int ledSuckPin = 9;
const int channel = 1;

int sendRate = 5;
int blowBrightness = 0;
int suckBrightness = 0;
int sensorMidpoint = 820;
int sensorNeedle = sensorMidpoint;
int sensorDeadzone = 30;
int n0 =0;

// store previously sent values, to detect changes
int previousA0 = -1;
elapsedMillis msec = 0;

void setup() {
  pinMode(ledBlowPin, OUTPUT);
  pinMode(ledSuckPin, OUTPUT);
  pinMode(ccSwitchPin, INPUT_PULLUP);
  analogWrite(ledBlowPin, brightness);
  analogWrite(ledSuckPin, brightness);
}

void loop() {
  // only check the analog inputs 50 times per second,
  // to prevent a flood of MIDI messages
  
  if(digitalRead(ccSwitchPin) == HIGH){
    blowCC = 2;
    suckCC = 11;
  } else {
    blowCC = 11;
    suckCC = 2;
  }
  
  if (msec >= sendRate) {
    msec = 0;
    n0 = analogRead(breathPin);
    sensorNeedle = n0 - sensorMidpoint;
    if (sensorNeedle < 0){
      blow();
    }
    if (sensorNeedle > 0 {
      suck();
    }
  }
  
  
  // MIDI Controllers should discard incoming MIDI messages.
  // http://forum.pjrc.com/threads/24179-Teensy-3-Ableton-Analog-CC-causes-midi-crash
  while (usbMIDI.read()) {
    // ignore incoming messages
  }
}

void blow(){
  if (abs(n0 - sensorMidpoint) >= sensorDeadzone){
    int value = constrain(map(n0, (sensorMidpoint + sensorDeadzone), 1023, 0, 127), 0, 127);
      // only transmit MIDI messages if analog input changed
      if (n0 != previousA0) {
        usbMIDI.sendControlChange(blowCC, value, channel);
        previousA0 = n0;
        blowBrightness = value * 2 ;
        Serial.println(value);
        analogWrite(ledBlowPin, blowBrightness);
      }
  } else {
    usbMIDI.sendControlChange(blowCC, 0, channel);
  }
}

void suck(){
  if (n0 - sensorMidpoint >= sensorDeadzone) {
  int value = constrain(map(n0, sensorMidpoint - sensorDeadzone, 0, 0, 127), 0, 127);
    // only transmit MIDI messages if analog input changed
    if (n0 != previousA0) {
      usbMIDI.sendControlChange(suckCC, value, channel);
      previousA0 = n0;
      suckBrightness = value * 2 ;
      Serial.println(value);
      analogWrite(ledSuckPin, suckBrightness);
    }
  }else {
    usbMIDI.sendControlChange(blowCC, 0, channel);
  }
}
