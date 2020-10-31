/*
  Analog Input
 Demonstrates analog input by reading an analog sensor on analog pin 0 and
 turning on and off a light emitting diode(LED)  connected to digital pin 13. 
 The amount of time the LED will be on and off depends on
 the value obtained by analogRead(). 
 
 The circuit:
 * Potentiometer attached to analog input 0
 * center pin of the potentiometer to the analog pin
 * one side pin (either one) to ground
 * the other side pin to +5V
 * LED anode (long leg) attached to digital output 13
 * LED cathode (short leg) attached to ground
 
 * Note: because most Arduinos have a built-in LED attached 
 to pin 13 on the board, the LED is optional.
 
 
 Created by David Cuartielles
 modified 30 Aug 2011
 By Tom Igoe
 
 This example code is in the public domain.
 
 http://arduino.cc/en/Tutorial/AnalogInput
 
 */

int joyXpin = 14;
int joyYpin = 20;
int ledPin = 13;      // select the pin for the LED

int XValue = 0;  // variable to store the value coming from the sensor
int YValue = 0;

void setup() {
  analogReadResolution(12);
  pinMode(ledPin, OUTPUT);  
}

void loop() {
  XValue = analogRead(joyXpin);    
  YValue = analogRead(joyYpin);
  Serial.println ("X");
  Serial.println (XValue);
  Serial.println ("Y");   
  Serial.println (YValue);
  digitalWrite(ledPin, HIGH);  
  delay(XValue/50);               
  digitalWrite(ledPin, LOW);   
  delay(YValue/50);                  
}
