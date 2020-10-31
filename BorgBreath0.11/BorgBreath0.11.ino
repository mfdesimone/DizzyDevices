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
float blowCurve = -7;

int sendRate = 5;
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
// store previously sent values, to detect changes
int previousA0 = -1;
int previousA1 = -1;
int previousBlowValue = 0;
int previousSuckValue = 0;
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
//        int scaledResult;
//        scaledResult = (int) fscale( 0, 127, 0, 127, value, blowCurve);

        usbMIDI.sendControlChange(blowCC, value, channel);
        previousA0 = n0;
        blowBrightness = value * 2 ;
        previousBlowValue = value;
        if (value < 5 ){
          value = 0;
          previousBlowValue = 0;
          usbMIDI.sendControlChange(blowCC, 0, channel);
          blowBrightness = 0;
        }
      }
      Serial.println(value);
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
/* fscale
 Floating Point Autoscale Function V0.1
 Paul Badger 2007
 Modified from code by Greg Shakar

 This function will scale one set of floating point numbers (range) to another set of floating point numbers (range)
 It has a "curve" parameter so that it can be made to favor either the end of the output. (Logarithmic mapping)

 It takes 6 parameters

 originalMin - the minimum value of the original range - this MUST be less than origninalMax
 originalMax - the maximum value of the original range - this MUST be greater than orginalMin

 newBegin - the end of the new range which maps to orginalMin - it can be smaller, or larger, than newEnd, to facilitate inverting the ranges
 newEnd - the end of the new range which maps to originalMax  - it can be larger, or smaller, than newBegin, to facilitate inverting the ranges

 inputValue - the variable for input that will mapped to the given ranges, this variable is constrained to originaMin <= inputValue <= originalMax
 curve - curve is the curve which can be made to favor either end of the output scale in the mapping. Parameters are from -10 to 10 with 0 being
          a linear mapping (which basically takes curve out of the equation)

 To understand the curve parameter do something like this:

 void loop(){
  for ( j=0; j < 200; j++){
    scaledResult = fscale( 0, 200, 0, 200, j, -1.5);

    Serial.print(j, DEC);  
    Serial.print("    ");  
    Serial.println(scaledResult, DEC);
  }  
}

And try some different values for the curve function - remember 0 is a neutral, linear mapping

To understand the inverting ranges, do something like this:

 void loop(){
  for ( j=0; j < 200; j++){
    scaledResult = fscale( 0, 200, 200, 0, j, 0);

    //  Serial.print lines as above

  }  
}

*/


//#include <math.h>
//
//int j;
//float scaledResult;
//
////void loop(){
////  for ( j=0; j < 128; j++){
////    scaledResult = fscale( 0, 127, 0, 127, j, 2);
////
////    Serial.print(j, DEC);  
////    Serial.print("    ");  
////    Serial.println(scaledResult , DEC);
////  }  
////}
//
//float fscale( float originalMin, float originalMax, float newBegin, float
//newEnd, float inputValue, float curve){
//
//  float OriginalRange = 0;
//  float NewRange = 0;
//  float zeroRefCurVal = 0;
//  float normalizedCurVal = 0;
//  float rangedValue = 0;
//  boolean invFlag = 0;
//
//
//  // condition curve parameter
//  // limit range
//
//  if (curve > 10) curve = 10;
//  if (curve < -10) curve = -10;
//
//  curve = (curve * -.1) ; // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output
//  curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function
//
//  /*
//   Serial.println(curve * 100, DEC);   // multply by 100 to preserve resolution  
//   Serial.println();
//   */
//
//  // Check for out of range inputValues
//  if (inputValue < originalMin) {
//    inputValue = originalMin;
//  }
//  if (inputValue > originalMax) {
//    inputValue = originalMax;
//  }
//
//  // Zero Refference the values
//  OriginalRange = originalMax - originalMin;
//
//  if (newEnd > newBegin){
//    NewRange = newEnd - newBegin;
//  }
//  else
//  {
//    NewRange = newBegin - newEnd;
//    invFlag = 1;
//  }
//
//  zeroRefCurVal = inputValue - originalMin;
//  normalizedCurVal  =  zeroRefCurVal / OriginalRange;   // normalize to 0 - 1 float
//
//  /*
//  Serial.print(OriginalRange, DEC);  
//   Serial.print("   ");  
//   Serial.print(NewRange, DEC);  
//   Serial.print("   ");  
//   Serial.println(zeroRefCurVal, DEC);  
//   Serial.println();  
//   */
//
//  // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine
//  if (originalMin > originalMax ) {
//    return 0;
//  }
//
//  if (invFlag == 0){
//    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;
//
//  }
//  else     // invert the ranges
//  {  
//    rangedValue =  newBegin - (pow(normalizedCurVal, curve) * NewRange);
//  }
//
//  return rangedValue;
//}
