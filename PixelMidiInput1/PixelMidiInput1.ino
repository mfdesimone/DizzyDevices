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
unsigned long previousMicros = 0; // Used to track how long between "bounces"
int WaitInterval = 60;// Interval, in microseconds to wait between dedupeLEDs

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  Serial.begin(115200);
}

void loop() {
  // usbMIDI.read() needs to be called rapidly from loop().  When
  // each MIDI messages arrives, it return true.  The message must
  // be fully processed before usbMIDI.read() is called again.
  if (usbMIDI.read()) {
    processMIDI();
  }
}


void processMIDI(void) {
  byte type, channel, data1, data2, cable; //declare these bytes so we can get them. See definitions below

  // fetch the MIDI message, defined by these 5 numbers (except SysEX)
  //
  type = usbMIDI.getType();       // which MIDI message, 128-255
  channel = usbMIDI.getChannel(); // which MIDI channel, 1-16
  data1 = usbMIDI.getData1();     // first data byte of message, 0-127
  data2 = usbMIDI.getData2();     // second data byte of message, 0-127
  cable = usbMIDI.getCable();     // which virtual cable with MIDIx8, 0-7

// Declare and define Mark's LED-related MIDI params
  int currentVel = 0; //The current note's velocity (volume) [0-127]
  int currentNote = 0; //The current note's Midi note number [0-127] 0 is C0, 1 is C#0, for example
  int pitchClass; //The current note's pitch class (0 is C 1, is C#, 2 is D, etc) [0-11]
  int currentHue; //The current note's LED Hue (color) [0-255]
  int currentSat; //The current note's LED Saturation (color intensity) [0-255]
  int currentVal; //The current note's LED Value (brightness [0-255]

  
  //Serial.print(": ");
  // print info about the message
  //
  switch (type) {
    case usbMIDI.NoteOff: // 0x80 -- this type of midi message is recieved when a specific midi note is turned off
      
      currentNote = data1; 
      dedupeLEDs(currentNote);
//      leds[currentNote] = CRGB::Black; 
      numberOfLedsOn--; // Count this LED as OFF
//      if (numberOfLedsOn > 5) {
//        FastLED.clear ();
//      }
      Serial.print("currentNote= ");
      Serial.print(currentNote);
      Serial.print("Note Off, ch= ");
      Serial.print(channel, DEC);
      Serial.print(", note= ");
      Serial.print(data1, DEC);
      Serial.print(", velocity= ");
      Serial.println(data2, DEC);
      FastLED.show();
      break;

    case usbMIDI.NoteOn:  // 0x90 -- this type of midi message is received when a specific midi note is turned on
      currentNote = data1; // Let's define the current note #
      pitchClass = currentNote % 12; // Let's figure out which note it is (C, D, E, etc)
      currentSat = 255 - (data2 * .7); // Let's decide how much of that pitch class's color to use (none is white, all is fully saturated with color)
      currentVal = data2 + 100; // Let's decide how bright the LED will be overall (note that the apparent brightness is affected by the currentSat value too)
      currentHue = hueIndex[pitchClass]; // Let's figure out which of the 12 predetermined colors in hueIndex this pitch should use
      leds[currentNote] = CHSV(currentHue,currentSat,currentVal); // Let's use the above information to turn on the right LED with the right values
      numberOfLedsOn ++; // Count this LED as ON
      Serial.print(", polyphony= ");
      Serial.print(numberOfLedsOn);
      Serial.print("currentNote= ");
      Serial.print(currentNote);
      Serial.print("Note On, ch= ");
      Serial.print(channel, DEC);
      Serial.print(", note= ");
      Serial.print(data1, DEC);
      Serial.print(", velocity= ");
      Serial.println(data2, DEC);
      FastLED.show();
      Serial.println(cable);
      break;

// I'm not currently using these less-common midi message types at the moment. I wonder what LED attributes could they correspond to.
//    case usbMIDI.AfterTouchPoly: // 0xA0
//      Serial.print("AfterTouch Change, ch=");
//      Serial.print(channel, DEC);
//      Serial.print(", note=");
//      Serial.print(data1, DEC);
//      Serial.print(", velocity=");
//      Serial.println(data2, DEC);
//      break;
//
//    case usbMIDI.ControlChange: // 0xB0
//      Serial.print("Control Change, ch=");
//      Serial.print(channel, DEC);
//      Serial.print(", control=");
//      Serial.print(data1, DEC);
//      Serial.print(", value=");
//      Serial.println(data2, DEC);
//      break;
//
//    case usbMIDI.ProgramChange: // 0xC0
//      Serial.print("Program Change, ch=");
//      Serial.print(channel, DEC);
//      Serial.print(", program=");
//      Serial.println(data1, DEC);
//      break;
//
//    case usbMIDI.AfterTouchChannel: // 0xD0
//      Serial.print("After Touch, ch=");
//      Serial.print(channel, DEC);
//      Serial.print(", pressure=");
//      Serial.println(data1, DEC);
//      break;
//
//    case usbMIDI.PitchBend: // 0xE0
//      Serial.print("Pitch Change, ch=");
//      Serial.print(channel, DEC);
//      Serial.print(", pitch=");
//      Serial.println(data1 + data2 * 128, DEC);
//      break;
//
//    case usbMIDI.SystemExclusive: // 0xF0
//      // Messages larger than usbMIDI's internal buffer are truncated.
//      // To receive large messages, you *must* use the 3-input function
//      // handler.  See InputFunctionsComplete for details.
//      Serial.print("SysEx Message: ");
//      printBytes(usbMIDI.getSysExArray(), data1 + data2 * 256);
//      Serial.println();
//      break;
//
//    case usbMIDI.TimeCodeQuarterFrame: // 0xF1
//      Serial.print("TimeCode, index=");
//      Serial.print(data1 >> 4, DEC);
//      Serial.print(", digit=");
//      Serial.println(data1 & 15, DEC);
//      break;
//
//    case usbMIDI.SongPosition: // 0xF2
//      Serial.print("Song Position, beat=");
//      Serial.println(data1 + data2 * 128);
//      break;
//
//    case usbMIDI.SongSelect: // 0xF3
//      Serial.print("Sond Select, song=");
//      Serial.println(data1, DEC);
//      break;
//
//    case usbMIDI.TuneRequest: // 0xF6
//      Serial.println("Tune Request");
//      break;
//
//    case usbMIDI.Clock: // 0xF8
//      Serial.println("Clock");
//      break;
//
//    case usbMIDI.Start: // 0xFA
//      Serial.println("Start");
//      break;
//
//    case usbMIDI.Continue: // 0xFB
//      Serial.println("Continue");
//      break;
//
//    case usbMIDI.Stop: // 0xFC
//      Serial.println("Stop");
//      break;
//
//    case usbMIDI.ActiveSensing: // 0xFE
//      Serial.println("Actvice Sensing");
//      break;
//
//    case usbMIDI.SystemReset: // 0xFF
//      Serial.println("System Reset");
//      break;

    default:
      Serial.println("Oops, an unknown MIDI message type!");
  }
}
void dedupeLEDs(int ledNum){
  unsigned long currentMicros = micros();
  if ((unsigned long)(currentMicros - previousMicros) >= WaitInterval) {
    leds[ledNum]= CRGB::Black;
  }
}

//
//void printBytes(const byte *data, unsigned int size) {
//  while (size > 0) {
//    byte b = *data++;
//    if (b < 16) Serial.print('0');
//    Serial.print(b, HEX);
//    if (size > 1) Serial.print(' ');
//    size = size - 1;
//  }
//}
