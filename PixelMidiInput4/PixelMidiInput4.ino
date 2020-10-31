/* Receive Incoming USB MIDI by reading data.  This approach
   gives you access to incoming MIDI message data, but requires
   more work to use that data.  For the simpler function-based
   approach, see InputFunctionsBasic and InputFunctionsComplete.

   Use the Arduino Serial Monitor to view the messages
   as Teensy receives them by USB MIDI

   You must select MIDI from the "Tools > USB Type" menu

   This example code is in the public domain.
*/
#include "FastLED.h"


#define NUM_LEDS 128 // How many leds in your strip?
#define USE_RUNNING_STATUS 1 //This is an attempt to account for a MIDI quirk called "RunningStatus"

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 6

CRGB leds[NUM_LEDS]; // Define the array of leds
volatile byte state = LOW; //for interrupt
int numberOfLedsOn = 0; //Track how many Leds are currently on
int hueIndex[] = {60, 64, 68, 90, 112, 160, 240, 250, 0, 6, 12, 25};
int switch_pin = 12;
int displayMode = 0;

void setup() {

  pinMode(switch_pin, INPUT_PULLUP);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  usbMIDI.setHandleNoteOff(OnNoteOff);
  usbMIDI.setHandleNoteOn(OnNoteOn);
  Serial.begin(115200);
}

void loop() {
  // usbMIDI.read() needs to be called rapidly from loop().  When
  // each MIDI messages arrives, it return true.  The message must
  // be fully processed before usbMIDI.read() is called again.
  usbMIDI.read();
}

void OnNoteOff(byte channel, byte data1, byte data2){
  delay(data1 * 0.05);
  int currentVel = 0; //The current note's velocity (volume) [0-127]
  int currentNote = 0; //The current note's Midi note number [0-127] 0 is C0, 1 is C#0, for example
  int pitchClass; //The current note's pitch class (0 is C 1, is C#, 2 is D, etc) [0-11]
  int currentHue; //The current note's LED Hue (color) [0-255]
  int currentSat; //The current note's LED Saturation (color intensity) [0-255]
  int currentVal; //The current note's LED Value (brightness [0-255] 
  displayMode = digitalRead(switch_pin);
  //Serial.print(displayMode);
  if (displayMode == 0){
    currentNote = ((data1 % 12) * 6) + int(data1/12); // Let's define the current note #
  };
  if (displayMode == 1){
    currentNote = data1; // Let's define the current note #
  };
  leds[currentNote] = CRGB::Black; 
  numberOfLedsOn--; // Count this LED as OFF
  if (numberOfLedsOn > 12) {
    FastLED.clear ();
  }
  FastLED.show();
}

void OnNoteOn(byte channel, byte data1, byte data2){
  //delay(data1 * 0.1);
  int currentVel = 0; //The current note's velocity (volume) [0-127]
  int currentNote = 0; //The current note's Midi note number [0-127] 0 is C0, 1 is C#0, for example
  int pitchClass; //The current note's pitch class (0 is C 1, is C#, 2 is D, etc) [0-11]
  int currentHue; //The current note's LED Hue (color) [0-255]
  int currentSat; //The current note's LED Saturation (color intensity) [0-255]
  int currentVal; //The current note's LED Value (brightness [0-255]
  displayMode = digitalRead(switch_pin);
  
  if (displayMode == 0){
    currentNote = ((data1 % 12) * 6) + int(data1/12); // Let's define the current note #
  };
  if (displayMode == 1){
    currentNote = data1; // Let's define the current note #
  };
  pitchClass = data1 % 12; // Let's figure out which note it is (C, D, E, etc)
  currentSat = 255 - (data2 * 1); // Let's decide how much of that pitch class's color to use (none is white, all is fully saturated with color)
  currentVal = data2 + 100; // Let's decide how bright the LED will be overall (note that the apparent brightness is affected by the currentSat value too)
  currentHue = hueIndex[pitchClass]; // Let's figure out which of the 12 predetermined colors in hueIndex this pitch should use
  leds[currentNote] = CHSV(currentHue,currentSat,currentVal); // Let's use the above information to turn on the right LED with the right values
  numberOfLedsOn ++; // Count this LED as ON

  FastLED.show();
}
