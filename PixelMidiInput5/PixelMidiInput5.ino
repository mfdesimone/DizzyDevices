
/*RGB Addressable LED Midi Visualizer

  This project was created by Mark DeSimone, using an example from https://www.pjrc.com/teensy/td_midi.html.

  Its original purpose is for controlling a 12 x 6 RGB LED  matrix similar to what's seen in the movie "Close Encounters of the Third Kind"
  but it should be adaptable to other configurations like the Adafruit Neopixel rings, or a simple strip of addressable LEDs - perhaps attached to a midi controller.
  It should work with a Teensy microcontroller, which can function as a USB Human interface device, and be visible as a USB midi device to your computer.

*/

#include "FastLED.h" //the FastLED library that drives all the LED data formatting. This project assumes you're using a strand of neoPixels


#define NUM_LEDS 72 // How many leds in your strip?
#define USE_RUNNING_STATUS 1 //This is an attempt to account for a MIDI quirk called "RunningStatus"

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 6

CRGB leds[NUM_LEDS]; // Define the array of leds
volatile byte state = LOW; //for interrupt
int numberOfLedsOn = 0; //Track how many Leds are currently on
int hueIndex[] = {60, 64, 68, 90, 112, 160, 240, 250, 0, 6, 12, 25}; // twelve hue values - one for each note in the chromatic scale
int switch_pin = 12; // connect a switch to this pin if you would like to switch between two display modes (vertical and horizontal, more or less)
int displayMode = 0;
int numRows = 6; // how many horizontal rows of LEDs do you have? This is only relevant if you're using displayMode 0 (see below)

void setup() {

  pinMode(switch_pin, INPUT_PULLUP); // Let's define the pinfor our DisplayMode switch
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS); //Let's set up the LEDs using the FastLED library, and tell it our LEDs are WS2812B. If you're using a different type of LED, you'll need to change WS2812B. If your LED strand's colors aren't in the order GRB, you can set that order on this line too.
  FastLED.clear(); // Let's clear any LED data that might be present
  usbMIDI.setHandleNoteOff(OnNoteOff); // Let's set up the usbMIDI library so it listens for MIDI Note On commands via midi
  usbMIDI.setHandleNoteOn(OnNoteOn); // Let's set up the usbMIDI library so it listens for MIDI Note Off commands via midi
  Serial.begin(115200); // Go serial go!
}

void loop() {
  // usbMIDI.read() needs to be called rapidly from loop().  When
  // each MIDI messages arrives, it return true.  The message must
  // be fully processed before usbMIDI.read() is called again.
  usbMIDI.read(); // every time this main loop runs, we'll read the usb input for MIDI commands.
}

void OnNoteOff(byte channel, byte data1, byte data2) { // this block of code is used any time the Arduino receives a noteOff command via usbMIDI.read()
  delay(data1 * 0.05); // this is added just as a way to prevent the Arduino from processing too many noteOff commands at once. there's probably a better way but this works pretty well

  int currentVel = 0; //The current note's velocity (volume) [0-127]
  int currentNote = 0; //The current note's Midi note number [0-127] 0 is C0, 1 is C#0, for example
  int pitchClass; //The current note's pitch class (0 is C 1, is C#, 2 is D, etc) [0-11]
  int currentHue; //The current note's LED Hue (color) [0-255]
  int currentSat; //The current note's LED Saturation (color intensity) [0-255]
  int currentVal; //The current note's LED Value (brightness [0-255]
  displayMode = digitalRead(switch_pin);
  //Serial.print(displayMode);
  if (displayMode == 0) {
    currentNote = ((data1 % 12) * numRows) + int(data1 / 12); // Let's define the current note #. This mode is useful if you are stringing twelve groups of LEDs vertically. set the numRows variable above to the number of rows you're using
  };
  if (displayMode == 1) {
    currentNote = data1; // Let's define the current note #. This mode is useful if you are stringing your LED rows horizontally, in groups of twelve.
  };
  leds[currentNote] = CRGB::Black; // this sets the current note's LED to black (off)
  numberOfLedsOn--; // Count this LED as OFF
  if (numberOfLedsOn > 7) {
    FastLED.clear (); // if there are more than 12 LEDs on, let's turn them all off. This is helpful in preventing 'stuck' LEDs - too many note commands at once can lead to some being ignored.
  }
  FastLED.show(); // this sends new data to the LED string.
}

void OnNoteOn(byte channel, byte data1, byte data2) { // this block of code is used any time the Arduino receives a noteOn cjommand via usbMIDI.read()
  //delay(data1 * 0.1);
  int currentVel = 0; //The current note's velocity (volume) [0-127]
  int currentNote = 0; //The current note's Midi note number [0-127] 0 is C0, 1 is C#0, for example
  int pitchClass; //The current note's pitch class (0 is C 1, is C#, 2 is D, etc) [0-11]
  int currentHue; //The current note's LED Hue (color) [0-255]
  int currentSat; //The current note's LED Saturation (color intensity) [0-255]
  int currentVal; //The current note's LED Value (brightness [0-255]
  displayMode = digitalRead(switch_pin); // let's make sure we're still using the correct displayMode

  if (displayMode == 0) {
    currentNote = ((data1 % 12) * 6) + int(data1 / 12); // Let's define the current note #. This mode is useful if you are stringing twelve groups of LEDs vertically. set the numRows variable above to the number of rows you're using
  };
  if (displayMode == 1) {
    currentNote = data1; // Let's define the current note #. This mode is useful if you are stringing your LED rows horizontally, in groups of twelve.
  };
  pitchClass = data1 % 12; // Let's figure out which note it is (C = 0, D = 2, E, etc). We don't care about octaves for this variable, so we use % to get the modulus of the currentNote divided by 12. data1 is the note's pitch number (0-127)
  currentSat = 255 - (data2 * 1); // Let's decide how much of that pitch class's color to use (none is white, all is fully saturated with color). data2 is the note's velocity (0-127)
  currentVal = data2 + 100; // Let's decide how bright the LED will be overall (note that the apparent brightness is affected by the currentSat value too)
  currentHue = hueIndex[pitchClass]; // Let's figure out which of the 12 predetermined colors in hueIndex this pitch should use. Take a look at the hueIndex array defined above for more info.
  leds[currentNote] = CHSV(currentHue, currentSat, currentVal); // Let's use the above information to turn on the right LED with the right values
  numberOfLedsOn ++; // Count this LED as ON

  FastLED.show(); // Let's send updated data to our LED string!
}
