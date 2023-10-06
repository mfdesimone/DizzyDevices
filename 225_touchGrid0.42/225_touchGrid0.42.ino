#include <Bounce2.h>
// for the build video that goes with this code, visit this link: 
// https://www.youtube.com/@markfdesimone/

// please see this instructable for the original code and inspiration behind the grid keyboard controller: 
// https://www.instructables.com/O-mat/

  
int RHoffset = 0; // this number will be added to notes on the right hand side of the key matrix. Set this higher if you need to be able to cover all 128 possible midi pitch numbers.
int octaveOffset = 2; // this number * 12 will be added to the note number sent by the device.
int noteVelMin = 30; // any note touch that registers below this midi velocity number will not be sent as a note-on message. adjust this number to suit your preference.
int rowInterval = 4; //5 = tritone in 12edo. 4 = perfect 4th.
int pressureThreshold = 180; // the minimum amount of pressure that should be handled as a touch.
int sensorMax = 400; // the maximum amount of pressure we care about. 1023 means max pressure / no resistance. So set this to 1023 to use the full range of sensitivity.
bool doAfterTouch = true; // whether polyphonic aftertouch CC should be sent
byte modeCheck = 0;
const byte pedal = 12; // pin number we're connecting to the sustain pedal
bool sustainIsOn = 0;
Bounce pedalBounce = Bounce(); //uses the bounce2 library to debounce a closed circuit from the pedal
int debounceInterval = 5; //milliseconds to wait during debounce

// Mux control pins for analog signal (SIG_pin) 
//********************
// NOTE! on the column-side multiplexer (mux), each channel between 0 and 13 must also connect to its own 1000 ohm resistor. 
// the other side of the resistor must connect to ground.
// the Enable pin on each of the multiplexers should be connected to ground.
// the Vcc pins on each multiplexers should be connected to 3.3volts on the microcontroller.
// no resistors are needed for the row-side multiplexer.
// the multiplexer's part number is CD74HC4067.

const byte s0 = A5; // connect the A5 pin to S0 on the column-side CD74HC4067 multiplexer
const byte s1 = A6; // connect the A6 pin to S1 on the column-side CD74HC4067 multiplexer
const byte s2 = A7; // connect the A7 pin to S2 on the column-side CD74HC4067 multiplexer
const byte s3 = A8; // connect the A8 pin to S3 on the column-side CD74HC4067 multiplexer

// Mux control pins for Output signal (OUT_pin) 
const byte w0 = 0; // connect the 0 pin to S0 on the row-side CD74HC4067 multiplexer
const byte w1 = 1; // connect the 1 pin to S1 on the row-side CD74HC4067 multiplexer
const byte w2 = 2; // connect the 2 pin to S2 on the row-side CD74HC4067 multiplexer
const byte w3 = 3; // connect the 3 pin to S3 on the row-side CD74HC4067 multiplexer

const byte SIG_pin = A9; //Mux in "SIG" pin on column-side multiplexer
const byte OUT_pin = 4; //Mux out "SIG" pin on row-side multiplexer

const boolean muxChannel[15][4]={ // these are the binary addresses that the microcontroller sends to each multiplexer to select the active channel. the addresses are sent by way of 4 output pins, one for each bit, set either low or high.
    {0,0,0,0}, //channel 0
    {1,0,0,0}, //channel 1
    {0,1,0,0}, //channel 2
    {1,1,0,0}, //channel 3
    {0,0,1,0}, //channel 4
    {1,0,1,0}, //channel 5
    {0,1,1,0}, //channel 6
    {1,1,1,0}, //channel 7
    {0,0,0,1}, //channel 8
    {1,0,0,1}, //channel 9
    {0,1,0,1}, //channel 10
    {1,1,0,1}, //channel 11
    {0,0,1,1}, //channel 12
    {1,0,1,1}, //channel 13
    {0,1,1,1} //channel 14
  };
  
//incoming serial byte
int inByte = 0;
int sensorReading = 0;     //variable for sending bytes to processing
int calibra[15][15];       //Calibration array for the min values of each od the 256 sensors.
int minsensor=254;         //Variable for staring the min array
bool thisNoteOn[15][15];   // track whether a sensor's assigned pitch is on or off.
byte thisNoteAftertouchValue [15][15] = { // keep track of each sensor's most recent polyphonic aftertouch value
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

void setup(){
  pinMode(pedal, INPUT_PULLUP);
  pinMode(s0, OUTPUT); 
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT); 
  pinMode(w0, OUTPUT); 
  pinMode(w1, OUTPUT); 
  pinMode(w2, OUTPUT); 
  pinMode(w3, OUTPUT); 
  pinMode(OUT_pin, OUTPUT); 

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  digitalWrite(w0, LOW);
  digitalWrite(w1, LOW);
  digitalWrite(w2, LOW);
  digitalWrite(w3, LOW);
  digitalWrite(OUT_pin, HIGH);
  Serial.begin(115200);
  calibration(); // this runs a function that will calibrate each of the 255 sensors. each sensor will be measured in its 'dormant' state, to establish the lowest voltage value we can expect from that sensor.
  pedalBounce.attach(pedal); // After setting up the button, setup the Bounce instance
  pedalBounce.interval(debounceInterval); // interval in ms
}
void loop(){
  while (usbMIDI.read()) {
    // ignore incoming messages because this device is designed for output only.
  }
//  readPedal(1); // input is the midi channel. this is commented out by default. If you add a sustain pedal jack to your project, uncomment this.
  for(int j = 0; j <= 14; j++){ //// SELECT A ROW
    writeMux(j); // sends a 4-bit signal to the row mux via the 4 output pins
    for(int i = 0; i <= 14; i++){ //// READ EACH OF THE COLUMNS' VOLTAGES for this row
      sensorReading = readMux(i);
      processReading(sensorReading, j, i);// figure out what to do with the reading we just took
    }
  }
}

// MIDI CALLBACKS -- these functions handle midi-related processes
void handleNoteOn(byte noteNumber, byte noteChannel, int processedPressure, int row, int column){ 
//  Serial.print("noteOn \t");
//  Serial.println(noteNumber); // there are Serial.print commands throughout this code that can be uncommented if you'd like to watch the serial monitor for testing/debugging. 

  noteNumber = constrain(noteNumber, 0, 127); // this ensures we don't send a pitch value outside of the valid range of 0-127.
  int noteVelocity = map(processedPressure, pressureThreshold, sensorMax,noteVelMin,127); // look up the map function for help with this line
  noteVelocity = constrain(noteVelocity, 0, 127); //// if the sensorReading falls between the minimum (calibra[j][i]) and sensorMax, let's map its value to the range of values we ultimately need.
  if(noteVelocity < noteVelMin){ // if the note velocity we've just calculated is below the noteVelMin (which is defined above), we 'return' which ends the current iteration of the handleNoteOn callback
    noteVelocity = 0;
    return;
  }
  if(noteVelocity > 127){ // this ensures we won't send a velocity value higher than 127
    noteVelocity = 127;
  }
//  Serial.print("vel \t");
//  Serial.print(noteVelocity);
//  Serial.print("noteNumber \t");
//  Serial.print(noteNumber);
//  Serial.print("noteChannel \t");
//  Serial.println(noteChannel);
  usbMIDI.sendNoteOn(noteNumber, noteVelocity, noteChannel);
}
void handleNoteOff(byte noteNumber, byte noteChannel){
//  Serial.print("noteOFF \t");
//  Serial.println(noteNumber);
//  Serial.print("noteOFF \t");
//  Serial.print("noteNumber \t");
//  Serial.print(noteNumber);
//  Serial.print("noteChannel \t");
//  Serial.println(noteChannel);
  usbMIDI.sendNoteOn(noteNumber, 0, noteChannel);
}
void handleAftertouch(byte noteNumber, byte noteChannel, int processedPressure, int row, int column){
  byte thisAfterTouch = map(processedPressure, pressureThreshold, sensorMax,noteVelMin,127); //// if the sensorReading falls between the minimum (calibra[j][i]) and sensorMax, let's map its value to the range of values we ultimately need.
  thisAfterTouch = constrain(thisAfterTouch, 0, 127);
  if(thisAfterTouch < noteVelMin){
    thisAfterTouch = 0;
    return;
  }
  if(thisAfterTouch > 127){
    thisAfterTouch = 127;
  }
//  Serial.print("thisAfterTouch \t");
//  Serial.println(thisAfterTouch);
  if (thisNoteAftertouchValue[row][column] == thisAfterTouch) {
//    Serial.print("same pressure \t");
//    Serial.println(processedPressure);
    return;
  } else {
    usbMIDI.sendAfterTouchPoly(noteNumber, thisAfterTouch, noteChannel);
//    Serial.print("Aftertouch note \t");
//    Serial.print(noteNumber);
//    Serial.print("\t Aftertouch value \t");
//    Serial.print(thisAfterTouch);
//    Serial.print("\t pressure \t");
//    Serial.println(processedPressure);
    thisNoteAftertouchValue[row][column] = thisAfterTouch;
  }
}

// BASE FUNCTIONS -- these functions are used by the midi callbacks above.
void processReading(int measuredPressure, int row, int column){
    //is reading above threshold?

    byte thisNote = row + column + (rowInterval*row) + (octaveOffset *12);
  if (column >7){    // if thisNote's column # is 8-14, we'll add the RHoffset value to its number
    thisNote = thisNote + RHoffset;
  }
  if (measuredPressure <= pressureThreshold && !thisNoteOn[row][column]){ // if the measured pressure is below the threshold we set, and if thisNote is not already set to 'on' in the thisNoteOn array, let's 'return' and end this iteration of the function
    return;
  }
  if (measuredPressure <= pressureThreshold && thisNoteOn[row][column]){ // if the measured pressure is below the threshold we set, but the note is marked as ON in the thisNoteOn array, that means the player let go of the key and we need to send a note-off message.
    handleNoteOff(thisNote, 1);
    thisNoteOn[row][column] = false;
    if (modeCheck > 0){
      if ((row == 0 && column == 1) || (row == 1 && column == 0)){
      Serial.println("Z");
      modeCheck=0;
      }
    }
    return;
  }
  if (measuredPressure>pressureThreshold){
//    Serial.print(measuredPressure);
//    Serial.print("\t");
//    Serial.print(column);
//    Serial.print("\t");
//    Serial.println(row);

    if (row == 0 && column == 1&& modeCheck == 0) { // this section checks whether the keys at 0,1 and 1,0 are both being pressed. 
    Serial.println("A");
    modeCheck = 1;
    }
    if (row == 1 && column == 0 && modeCheck ==1){
      Serial.println("B");
      modeCheck = 2;
    }
    if (modeCheck == 2) { //If both of these keys are being pressed, the switch() below will allow us to change instrument settings (until it's powered off)
      switch (column){
        case 14: 
          rowInterval = row;
//          Serial.println(rowInterval);
          break;
        case 13:
          RHoffset = row;
          if (row > 0){
            RHoffset = RHoffset + 3;
          }
//          Serial.println(RHoffset);
          break;
        case 12:
          octaveOffset = row;
          break;
        case 11:
          pressureThreshold = (row + 10) * 10 + 22;
//          Serial.println(pressureThreshold);
          break;
        case 10:
          sensorMax = (row + 15) * 18 + 25;
//          Serial.println(sensorMax);
          break;
        case 9: 
          if (row == 0){
            doAfterTouch = false;
          }
          if (row == 1){
            doAfterTouch = true;
          }
          break;
      }
    }
    if (!thisNoteOn[row][column]){
      int sensorNow = analogRead(SIG_pin);
      if (sensorNow > measuredPressure) {measuredPressure = sensorNow;}
//      sensorNow = analogRead(SIG_pin);
//      if (sensorNow > measuredPressure) {measuredPressure = sensorNow;};
       handleNoteOn(thisNote, 1, measuredPressure, row, column);
      // record that the note is on
      thisNoteOn[row][column] = true;
    }
    if (thisNoteOn[row][column] && doAfterTouch){
      //check if aftertouch value has changed. If so, send a new value for this note
      handleAftertouch(thisNote, 1, measuredPressure, row, column);
    }
  }
}

void calibration(){
  Serial.println("Calibrating...");
  // Full of 0's of initial matrix
  for(byte j = 0; j <= 14; j ++){ 
    writeMux(j);
    for(byte i = 0; i <= 14; i ++)
      calibra[j][i] = 0;
  }
  // Calibration
  for(byte k = 0; k < 50; k++){  
    for(byte j = 0; j <= 14; j ++){ 
      writeMux(j);
      for(byte i = 0; i <= 14; i ++)
        calibra[j][i] = calibra[j][i] + readMux(i);
    }
  }
  //Print averages
  for(byte j = 0; j <= 14; j ++){ 
    writeMux(j);
    for(byte i = 0; i < 14; i ++){
      calibra[j][i] = calibra[j][i]/50;
      if(calibra[j][i] < minsensor) //// if the measured calibration is less than the default minimum value, reset the minimum to match the calibration #
        minsensor = calibra[j][i];
      Serial.print(calibra[j][i]);
      Serial.print("\t");
    }
  Serial.println(); 
  }
  Serial.println();
  Serial.print("Minimum Value: ");
  Serial.println(minsensor);

}
int readMux(byte channel){ //// READ THE COLUMNS TO FIND OUT HOW MUCH VOLTAGE EACH COLUMN IS GETTING
  byte controlPin[] = {s0, s1, s2, s3};
  //loop through the 4 sig
  for(int i = 0; i < 4; i ++){ //// these control the columns' analog input mux'er (the analog voltage input from the matrix)
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }
  //read the value at the SIG pin
  int val = analogRead(SIG_pin);

  //return the value
  return val;
}
void writeMux(byte channel){ //// SELECT THE ROWS AND SEND VOLTAGE THROUGH EACH ONE AT A TIME
  byte controlPin[] = {w0, w1, w2, w3}; ////these control the row mux'er (the digital voltage output to the matrix). Does not collect data. Just selects current row mux channel
  //loop through the 4 sig
  for(byte i = 0; i < 4; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }
}

void readPedal(byte noteChannel){ // this function figures out if the pedal is was just pressed or lifted. If it detects that the state has changed from one to the other, it will send the appropriate midi command.
  pedalBounce.update();
  if (pedalBounce.fell()){
    usbMIDI.sendControlChange(64, 127, noteChannel);
    sustainIsOn = true;
//    Serial.println("fell");
  }
    if (pedalBounce.rose() ){
    usbMIDI.sendControlChange(64, 0, noteChannel);
    sustainIsOn = false;
//    Serial.println("rose");
  }
}
