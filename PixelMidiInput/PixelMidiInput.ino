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

// How many leds in your strip?
#define NUM_LEDS 128
#define USE_RUNNING_STATUS 1 

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 6

// Define the array of leds
CRGB leds[NUM_LEDS];
volatile byte state = LOW; //for interrupt
int numberOfLedsOn = 0;

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(DATA_PIN), blink, HIGH);
}

void loop() {
  // usbMIDI.read() needs to be called rapidly from loop().  When
  // each MIDI messages arrives, it return true.  The message must
  // be fully processed before usbMIDI.read() is called again.
  if (usbMIDI.read()) {
    processMIDI();
  }
}
void blink() { // used for the interrupt
  state = !state;
}

void processMIDI(void) {
  byte type, channel, data1, data2, cable;

  // fetch the MIDI message, defined by these 5 numbers (except SysEX)
  //
  type = usbMIDI.getType();       // which MIDI message, 128-255
  channel = usbMIDI.getChannel(); // which MIDI channel, 1-16
  data1 = usbMIDI.getData1();     // first data byte of message, 0-127
  data2 = usbMIDI.getData2();     // second data byte of message, 0-127
  cable = usbMIDI.getCable();     // which virtual cable with MIDIx8, 0-7

    //Serial.print(": ");

  // print info about the message
  //
  int currentVel = 0;
  int currentNote = 0;
  switch (type) {
    case usbMIDI.NoteOff: // 0x80
      currentNote = data1;
      Serial.print("currentNote");
      Serial.print(currentNote);
      Serial.print("Note Off, ch=");
      Serial.print(channel, DEC);
      Serial.print(", note=");
      Serial.print(data1, DEC);
      Serial.print(", velocity=");
      Serial.println(data2, DEC);
      leds[currentNote] = CRGB::Black; 
      FastLED.show();
//      numberOfLedsOn--;
//      if (numberOfLedsOn > 5) {
//        FastLED.clear ();
//      }
      break;

    case usbMIDI.NoteOn: // 0x90
      currentNote = data1;
      Serial.print("currentNote");
      Serial.print(currentNote);
      Serial.print("Note On, ch=");
      Serial.print(channel, DEC);
      Serial.print(", note=");
      Serial.print(data1, DEC);
      Serial.print(", velocity=");
      Serial.println(data2, DEC);
      leds[currentNote] = CRGB::Blue;
      FastLED.show();
//      numberOfLedsOn ++;
      //Serial.print(", polyphony=");
      //Serial.print(numberOfLedsOn);
      Serial.println(cable);
      break;

    case usbMIDI.AfterTouchPoly: // 0xA0
      Serial.print("AfterTouch Change, ch=");
      Serial.print(channel, DEC);
      Serial.print(", note=");
      Serial.print(data1, DEC);
      Serial.print(", velocity=");
      Serial.println(data2, DEC);
      break;

    case usbMIDI.ControlChange: // 0xB0
      Serial.print("Control Change, ch=");
      Serial.print(channel, DEC);
      Serial.print(", control=");
      Serial.print(data1, DEC);
      Serial.print(", value=");
      Serial.println(data2, DEC);
      break;

    case usbMIDI.ProgramChange: // 0xC0
      Serial.print("Program Change, ch=");
      Serial.print(channel, DEC);
      Serial.print(", program=");
      Serial.println(data1, DEC);
      break;

    case usbMIDI.AfterTouchChannel: // 0xD0
      Serial.print("After Touch, ch=");
      Serial.print(channel, DEC);
      Serial.print(", pressure=");
      Serial.println(data1, DEC);
      break;

    case usbMIDI.PitchBend: // 0xE0
      Serial.print("Pitch Change, ch=");
      Serial.print(channel, DEC);
      Serial.print(", pitch=");
      Serial.println(data1 + data2 * 128, DEC);
      break;

    case usbMIDI.SystemExclusive: // 0xF0
      // Messages larger than usbMIDI's internal buffer are truncated.
      // To receive large messages, you *must* use the 3-input function
      // handler.  See InputFunctionsComplete for details.
      Serial.print("SysEx Message: ");
      printBytes(usbMIDI.getSysExArray(), data1 + data2 * 256);
      Serial.println();
      break;

    case usbMIDI.TimeCodeQuarterFrame: // 0xF1
      Serial.print("TimeCode, index=");
      Serial.print(data1 >> 4, DEC);
      Serial.print(", digit=");
      Serial.println(data1 & 15, DEC);
      break;

    case usbMIDI.SongPosition: // 0xF2
      Serial.print("Song Position, beat=");
      Serial.println(data1 + data2 * 128);
      break;

    case usbMIDI.SongSelect: // 0xF3
      Serial.print("Sond Select, song=");
      Serial.println(data1, DEC);
      break;

    case usbMIDI.TuneRequest: // 0xF6
      Serial.println("Tune Request");
      break;

    case usbMIDI.Clock: // 0xF8
      Serial.println("Clock");
      break;

    case usbMIDI.Start: // 0xFA
      Serial.println("Start");
      break;

    case usbMIDI.Continue: // 0xFB
      Serial.println("Continue");
      break;

    case usbMIDI.Stop: // 0xFC
      Serial.println("Stop");
      break;

    case usbMIDI.ActiveSensing: // 0xFE
      Serial.println("Actvice Sensing");
      break;

    case usbMIDI.SystemReset: // 0xFF
      Serial.println("System Reset");
      break;

    default:
      Serial.println("Opps, an unknown MIDI message type!");
  }
}


void printBytes(const byte *data, unsigned int size) {
  while (size > 0) {
    byte b = *data++;
    if (b < 16) Serial.print('0');
    Serial.print(b, HEX);
    if (size > 1) Serial.print(' ');
    size = size - 1;
  }
}
