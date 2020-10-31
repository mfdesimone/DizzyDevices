/* USB MIDI AnalogControlChange Example

   You must select MIDI from the "Tools > USB Type" menu
   http://www.pjrc.com/teensy/td_midi.html

   This example code is in the public domain.
*/

//#include <Bounce.h>


// the MIDI continuous controller for each analog input
int blowCC = 2; // Breath CC
int suckCC = 11;
int volumeCC = 7;
const int breathPin = 15;
const int volumePin = 14;
const int ccSwitchPin = 11;
const int ledBlowPin = 10;
const int ledSuckPin = 9;
const int channel = 1;

int sendRate = 3;
int blowBrightness = 0;
int suckBrightness = 0;
int sensorMidpoint = 820;
int sensorNeedle = sensorMidpoint;
int sensorDeadzone = 80;
int volumeDeadzone = 2;
int n0 = 0;
int n1 = 0;

int breathCalibrationMeasurement = 820;
int breathCalibrationTotal = 0;
bool calibrated = false;
bool notBlowing = true;
// store previously sent values, to detect changes
int previousA0 = -1;
int previousA1 = -1;
int previousBlowValue = 0;
int previousSuckValue = 0;
int valueToSend = 0;
int startingArticulationValue = 20;
int articulationHelper = startingArticulationValue;

elapsedMillis msec = 0;

void setup() {
  pinMode(ledBlowPin, OUTPUT);
  pinMode(ledSuckPin, OUTPUT);
  pinMode(ccSwitchPin, INPUT_PULLUP);
  analogWrite(ledBlowPin, blowBrightness);
  analogWrite(ledSuckPin, suckBrightness);

}

void loop() {
  // only check the analog inputs 50 times per second,
  // to prevent a flood of MIDI messages
  if(calibrated == false){
    Serial.println("calibrating!");
    calibrateBreath();
  }

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
    n1 = analogRead(volumePin) / 8;

    sensorNeedle = n0 - sensorMidpoint;
//    Serial.println(sensorNeedle);
    if (sensorNeedle < 0){
      blow();
    }
    if (sensorNeedle > 0) {
      suck();
    }
    if (abs(n1 - previousA1) > volumeDeadzone) {
      usbMIDI.sendControlChange(volumeCC, n1, channel);
      previousA1 = n1;
      Serial.println(n1);
    }
  }
  goLED();
  
  // MIDI Controllers should discard incoming MIDI messages.
  // http://forum.pjrc.com/threads/24179-Teensy-3-Ableton-Analog-CC-causes-midi-crash
  while (usbMIDI.read()) {
    // ignore incoming messages
  }
}

void blow(){
  if (abs(sensorNeedle) >= sensorDeadzone){
//    int value = constrain(map(n0, (sensorMidpoint + sensorDeadzone), 1023, 0, 127), 0, 127);
      int value = constrain(map(n0, sensorMidpoint - sensorDeadzone, 0, 0, 127), 0, 127);
      // only transmit MIDI messages if analog input changed
      if (value != previousBlowValue) {
        valueToSend = value + articulationHelper;
        if (articulationHelper >=0) {
          articulationHelper --;
        }
        usbMIDI.sendControlChange(blowCC, valueToSend, channel);
        previousA0 = n0;
        blowBrightness = value * 2 ;
        previousBlowValue = value;
        if (value < 10 ){
          value = 0;
          articulationHelper = startingArticulationValue;
          previousBlowValue = 0;
          usbMIDI.sendControlChange(blowCC, 0, channel);
          blowBrightness = 0;
        }
      Serial.println(value);
    }
  } 
}

void suck(){
  if (sensorNeedle >= sensorDeadzone) {
//  int value = constrain(map(n0, sensorMidpoint - sensorDeadzone, 0, 0, 127), 0, 127);
    int value = constrain(map(n0, (sensorMidpoint + sensorDeadzone), 1023, 0, 127), 0, 127);
    // only transmit MIDI messages if analog input changed
    if (value != previousSuckValue) {
      usbMIDI.sendControlChange(suckCC, value, channel);
      previousA0 = n0;
      suckBrightness = value * 2 ;
      Serial.println(value);
      previousSuckValue = value;

      if (value < 15 ){
        value = 0;
        previousSuckValue = 0;
        usbMIDI.sendControlChange(suckCC, 0, channel);
        suckBrightness = 0;
      }
    }
    Serial.println(value);
  }
}
void goLED(){
  analogWrite(ledBlowPin, blowBrightness);
  analogWrite(ledSuckPin, suckBrightness);
}
 
void calibrateBreath(){
  delay(400);
    for (int i = 0; i <= 10; i++) {
    breathCalibrationMeasurement = analogRead(breathPin);
    Serial.println(breathCalibrationMeasurement);
    breathCalibrationTotal += breathCalibrationMeasurement;  
    if (i % 2 == 0){
      analogWrite(ledBlowPin, 255);
      analogWrite(ledSuckPin, 0);
    } else if (i % 2 == 1){
      analogWrite(ledBlowPin, 0);
      analogWrite(ledSuckPin, 255);
    }
    delay(20);
    if (i == 10) {
      sensorMidpoint = breathCalibrationTotal / 11;
      calibrated = true;
        Serial.println("Calibrated!!!");
        Serial.println(sensorMidpoint);
        analogWrite(ledBlowPin, 0);
        analogWrite(ledSuckPin, 0);
    }
  }
 }
