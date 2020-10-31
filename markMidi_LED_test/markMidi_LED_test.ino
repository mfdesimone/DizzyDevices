/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink
*/
int LED1 = 2;
int LED2 = 3;
int LED3 = 4;
int LED4 = A3;
int LED5 = A4;
int Knob1 = A0;
int sensorValue = 0;
int delInterval = 100;
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  pinMode(Knob1, INPUT);
}

// the loop function runs over and over again forever
void loop() {
  sensorValue = analogRead(Knob1);
  digitalWrite(LED1, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(sensorValue);                       // wait for a second
  digitalWrite(LED1, LOW);    // turn the LED off by making the voltage LOW
  delay(sensorValue);                       // wait for a second
  digitalWrite(LED2, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(sensorValue);                       // wait for a second
  digitalWrite(LED2, LOW);    // turn the LED off by making the voltage LOW
  delay(sensorValue);                       // wait for a second
  digitalWrite(LED3, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(sensorValue);                       // wait for a second
  digitalWrite(LED3, LOW);    // turn the LED off by making the voltage LOW
  delay(sensorValue);                       // wait for a second
  digitalWrite(LED4, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(sensorValue);                       // wait for a second
  digitalWrite(LED4, LOW);    // turn the LED off by making the voltage LOW
  delay(sensorValue);                       // wait for a second
  digitalWrite(LED5, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(sensorValue);                       // wait for a second
  digitalWrite(LED5, LOW);    // turn the LED off by making the voltage LOW
  delay(sensorValue);                       // wait for a second
}
