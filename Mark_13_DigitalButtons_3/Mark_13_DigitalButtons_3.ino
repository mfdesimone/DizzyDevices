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

const int b_0 = 0;
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
const int b_page = 1;
//const int LED1 = A5;
//const int LED2 = A4;
const int ledPin =  13;      // the number of the onboard LED pin

//int LED1State = 0;
//int LED2State = 0;

int i = 0;
void setup() {
  //start serial connection
  Serial.begin(9600);
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
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
//  pinMode(A5, OUTPUT); //LED 1
//  pinMode(A4, OUTPUT); //LED 2
  pinMode(ledPin, OUTPUT); 

}

void loop() {

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
  
  if (buttonState_b_0 == LOW) {
    Serial.println("b_0");
  }
  
  if (buttonState_b_1 == LOW) {
    Serial.println("b_1");
  }
  
  if (buttonState_b_2 == LOW) {
    Serial.println("b_2");
  }
  
  if (buttonState_b_3 == LOW) {
    Serial.println("b_3");
  }
  
  if (buttonState_b_4 == LOW) {
    Serial.println("b_4");
  }

  if (buttonState_b_5 == LOW) {
    Serial.println("b_5");
  }

  if (buttonState_b_6 == LOW) {
    Serial.println("b_6");
  }

  if (buttonState_b_7 == LOW) {
    Serial.println("b_7");
  }

  if (buttonState_b_8 == LOW) {
    Serial.println("b_8");
  }

  if (buttonState_b_9 == LOW) {
    Serial.println("b_9");
  }

  if (buttonState_b_10 == LOW) {
    Serial.println("b_10");
  }

  if (buttonState_b_11 == LOW) {
    Serial.println("b_11");
  }

  if (buttonState_b_12 == LOW) {
    Serial.println("b_12");
  }

  if (buttonState_b_func == LOW) {
    Serial.println("b_func");
  }

     
  delay (10);        // delay in between reads for stability
}

