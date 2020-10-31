/*
  Input Pull-up Serial

  This example demonstrates the use of pinMode(INPUT_PULLUP). It reads a digital
  input on pin 2 and prints the results to the Serial Monitor.

  The circuit:
  - momentary switch attached from pin 2 to ground
  - built-in LED on pin 13

  Unlike pinMode(INPUT), there is no pull-down resistor necessary. An internal
  20K-ohm resistor is pulled to 5V. This configuration causes the input to read
  HIGH when the switch is open, and LOW when it is closed.

  created 14 Mar 2012
  by Scott Fitzgerald

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/InputPullupSerial
*/
// Setup pins for input

const int b_0 = A1;
const int b_1 = 2;
const int b_2 = 4;
const int b_3 = 3;
const int b_4 = 5;
const int b_5 = 7;
const int b_6 = 8;
const int b_7 = 6;
const int b_8 = 10;
const int b_9 = 9;
const int b_10 = A2;
const int b_11 = 12;
const int b_12 = 11;
const int b_func = A3;
const int b_page = A0;
const int LED1 = A5; // green
const int LED2 = A4; // red
const int ledPin =  13;      // the number of the onboard LED pin

int pageMode = 0; // 0 is off, 1 is on
int pageNum = 3; // this indcates the page number for notes
int buttonNum = 0; 
int channel = 0x90;
int velocity = 0x45;

void setup() {
  //start serial connection
  Serial.begin(31250);
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);  
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A5, OUTPUT); //LED 1
  pinMode(A4, OUTPUT); //LED 2
  pinMode(ledPin, OUTPUT); 

}
 int pitch( int x, int y ){ // x = pageNum; y = buttonNum
    int result = x * 12 + y; 
    return result; //let's calculate the pitch number to send. each page number * 12 is an octave
  }


void loop() { // main program loop

  int buttonState_b_0 = digitalRead(b_0);
  int buttonState_b_1 = digitalRead(b_1);
  int buttonState_b_2 = digitalRead(b_2);
  int buttonState_b_3 = digitalRead(b_3);
  int buttonState_b_4 = digitalRead(b_4);
  int buttonState_b_5 = digitalRead(b_5);
  int buttonState_b_6 = digitalRead(b_6);
  int buttonState_b_7 = digitalRead(b_7);
  int buttonState_b_8 = digitalRead(b_8);  
  int buttonState_b_9 = digitalRead(b_9);
  int buttonState_b_10 = digitalRead(b_10);
  int buttonState_b_11 = digitalRead(b_11);
  int buttonState_b_12 = digitalRead(b_12);
  int buttonState_b_func = digitalRead(b_func);  
  int buttonState_b_page = digitalRead(b_page);

  if (pageMode == 0) { // if we are not currently selecting a page
    if (buttonState_b_0 == LOW) {
      buttonNum = 0;
      int note = pitch(pageNum, buttonNum);
      noteEvent(note);
      //Serial.println("b_0");
    }
    if (buttonState_b_1 == LOW) {
      buttonNum = 1;
      int note = pitch(pageNum, buttonNum);
      noteEvent(note);
      //Serial.println("b_1");
    }
    if (buttonState_b_2 == LOW) {
      buttonNum = 2;
      int note = pitch(pageNum, buttonNum);
      noteEvent(note);
      //Serial.println("b_2");
    }
    if (buttonState_b_3 == LOW) {
      buttonNum = 3;
      int note = pitch(pageNum, buttonNum);
      noteEvent(note);
      //Serial.println("b_3");
    }
    if (buttonState_b_4 == LOW) {
      buttonNum = 4;
      int note = pitch(pageNum, buttonNum);
      noteEvent(note);
      //Serial.println("b_4");
    }
    if (buttonState_b_5 == LOW) {
      buttonNum = 5;
      int note = pitch(pageNum, buttonNum);
      noteEvent(note);
      //Serial.println("b_5");
    }
    if (buttonState_b_6 == LOW) {
      buttonNum = 6;
      int note = pitch(pageNum, buttonNum);       
      noteEvent(note);
      //Serial.println("b_6");
    }
    if (buttonState_b_7 == LOW) {
      buttonNum = 7;
      int note = pitch(pageNum, buttonNum); 
      noteEvent(note);
      //Serial.println("b_7");
    }
    if (buttonState_b_8 == LOW) {
      buttonNum = 8;
      int note = pitch(pageNum, buttonNum); 
      noteEvent(note);
      //Serial.println("b_8");
    }
    if (buttonState_b_9 == LOW) {
      buttonNum = 9;
      int note = pitch(pageNum, buttonNum); 
      noteEvent(note);
      //Serial.println("b_9");
    }
    if (buttonState_b_10 == LOW) {
      buttonNum = 10;
      int note = pitch(pageNum, buttonNum); 
      noteEvent(note);
      //Serial.println("b_10");
    }
    if (buttonState_b_11 == LOW) {
      buttonNum = 11;
      int note = pitch(pageNum, buttonNum); 
      noteEvent(note);
      //Serial.println("b_11");
    }
    if (buttonState_b_12 == LOW) {
      buttonNum = 12;
      int note = pitch(pageNum, buttonNum); 
      noteEvent(note);
      //Serial.println("b_12");
    }
    

  } 


// ****** Page Select ******
  if (pageMode == 1) { // if we are selecting a page
    if (buttonState_b_0 == LOW) {
      pageMode = 0; // now that we've selected this page, let's turn off pageMode
      pageNum = 0; // and let's remember the page number
      //Serial.println("page " + pageNum );
    }
    if (buttonState_b_1 == LOW) {
      pageMode = 0;
      pageNum = 1;
      //Serial.println("page " + pageNum );
    }
    if (buttonState_b_2 == LOW) {
      pageMode = 0;
      pageNum = 2;
      //Serial.println("page " + pageNum );
    }
    if (buttonState_b_3 == LOW) {
      pageMode = 0;
      pageNum = 3;
      //Serial.println("page " + pageNum );
    }
    if (buttonState_b_4 == LOW) {
      pageMode = 0;
      pageNum = 4;
      //Serial.println("page " + pageNum );
    }
    if (buttonState_b_5 == LOW) {
      pageMode = 0;
      pageNum = 5;
      //Serial.println("page " + pageNum );
    }
    if (buttonState_b_6 == LOW) {
      pageMode = 0;
      pageNum = 6;
      //Serial.println("page " + pageNum );
    }
    if (buttonState_b_7 == LOW) {
      pageMode = 0;
      pageNum = 7;
      //Serial.println("page " + pageNum );
    }
    if (buttonState_b_8 == LOW) {
      pageMode = 0;
      pageNum = 8;
      //Serial.println("page " + pageNum );
    }
    if (buttonState_b_9 == LOW) {
      pageMode = 0;
      pageNum = 9;
      //Serial.println("page " + pageNum );
    }
    if (buttonState_b_10 == LOW) {
      pageMode = 0;
      pageNum = 10;
      //Serial.println("page " + pageNum );
    }
    if (buttonState_b_11 == LOW) {
      pageMode = 0;
      pageNum = 11;
      //Serial.println("page " + pageNum );
    }
    if (buttonState_b_12 == LOW) {
      pageMode = 0;
      pageNum = 12;
      //Serial.println("page " + pageNum );
    }

  }
  
  if (buttonState_b_page == LOW) { // toggle the page mode on & off
    //Serial.println("b_page");
    if (pageMode == 0) {
      pageMode = 1;
      delay(200);
    } else if (pageMode == 1) {
      pageMode = 0;
    }
    
  }

  if (buttonState_b_func == LOW) {
    
    //Serial.println("b_func");
  }
  if (pageMode == 1) {
    digitalWrite(LED1, HIGH); 
  }
  else {
    digitalWrite(LED1, LOW);
  }
  delay (10);        // delay in between reads for stability
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



void noteEvent(int pitchNum) {
    //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
    noteOn(channel, pitchNum, velocity);
    delay(1);
    //Note on channel 1 (0x90), some note value (note), silent velocity (0x00):
    noteOff(channel, pitchNum, 0x00);
    delay(1);
}
