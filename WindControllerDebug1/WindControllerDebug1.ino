/**
 * Copyright (c) 2016, Gordon S. Good (velo27 [at] yahoo [dot] com)
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * The author's name may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL GORDON S. GOOD BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Wire.h>
#include "Adafruit_MPR121.h"

#define MIDI_CHANNEL 1
// The threshold level for sending a note on event. If the
// sensor is producing a level above this, we should be sounding
// a note.
#define NOTE_ON_THRESHOLD 300
// The maximum raw pressure value you can generate by
// blowing into the tube.
#define MAX_PRESSURE 500

// The three states of our state machine
// No note is sounding
#define NOTE_OFF 1
// We've observed a transition from below to above the
// threshold value. We wait a while to see how fast the
// breath velocity is increasing
#define RISE_WAIT 2
// A note is sounding
#define NOTE_ON 3
// Send aftertouch data no more than every AT_INTERVAL
// milliseconds
#define AT_INTERVAL 70
// We wait for 10 milliseconds of continuous breath
// pressure above NOTE+ON_THRESHOLD before we turn on
// the note, to de-glitch
#define RISE_TIME 3

// We keep track of which note is sounding, so we know
// which note to turn off when breath stops.
int noteSounding;
// The value read from the sensor
int sensorValue;
// The state of our state machine
int state;
// The time that we noticed the breath off -> on transition
unsigned long breath_on_time = 0L;
// The breath value at the time we observed the transition
int initial_breath_value;
// The aftertouch value we will send
int atVal;
// The last time we sent an aftertouch value
unsigned long atSendTime = 0L;

// This is the 12-input touch sensor
Adafruit_MPR121 touchSensor = Adafruit_MPR121();

// The total size of our fingering map (FMAP)
#define FMAP_SIZE 33

void setup() {
  while (!Serial); // Wait for serial to be ready
  
  Serial.begin(9600);
  Serial.println("Gordophone touch-sensitive recorder instrument sketch");

  // initialize state machine
  state = NOTE_OFF;
  
  // Set up touch sensor
  if (!touchSensor.begin(0x5A)) {
    Serial.println("MPR121 initialization failed");
    while (1);
  }
  Serial.println("MPR121 initialized");

  // Set up LEDs that will reflect touch state of instrument keys
  for (int i = 0; i < 12; i++) {
    pinMode(i, OUTPUT);
  }
}


// A mapping from a bitmask of fingering values to a MIDI note.
// The "keys" member is the bitmask of key-down values, and
// midi_note is the MIDI note to send when that set of
// keys are depressed.
struct fmap_entry {
  uint16_t keys;
  uint8_t midi_note;
};

// The table of valid fingering -> MIDI note mappings
// There is one entry in this table for every valid
// fingering combination.
struct fmap_entry fmap[FMAP_SIZE] = {
  // Middle octave
  // C
  {0b1111011100, 60},
  // Skip C# for now - key layout doesn't match recorder layout
  // D
  {0b0111011100, 62},
  // Skip D#
  // E
  {0b0011011100, 64},
  // F - note that there are two valid fingerings that will
  // produce an F. The first one is the "correct" one, and
  // the second one is how I learned to play recorder :-)
  {0b1101011100, 65},
  {0b0001011100, 65},
  // F#
  {0b0110011100, 66},
  // G
  {0b0000011100, 67},
  // Skip G# - key layout doesn't match recorder layout
  // A
  {0b0000001100, 69},
  // A#
  {0b0001010100, 70},
  // B
  {0b0000000100, 71},
  // C
  {0b0000001000, 72},

  // Lower octave - Least Significant Bit (LSB) is on.
  // C
  {0b1111011101, 48},
  // Skip C#
  // D
  {0b0111011101, 50},
  // Skip D#
  // E
  {0b0011011101, 52},
  // F
  {0b1101011101, 53},
  {0b0001011101, 53},  // Lame F
  // F#
  {0b0110011101, 54},
  // G
  {0b0000011101, 55},
  // Skip G#
  // A
  {0b0000001101, 57},
  // A#
  {0b0001010101, 58},
  // B
  {0b0000000101, 59},
  // C
  {0b0000001001, 60},

  
  // Upper octave - bit 1 is on, LSB is off
  // C
  {0b1111011110, 72},
  // Skip C#
  // D
  {0b0111011110, 74},
  // Skip D#
  // E
  {0b0011011110, 76},
  // F
  {0b1101011110, 77},
  {0b0001011110, 77},  // Lame F
  // F#
  {0b0110011110, 78},
  // G
  {0b0000011110, 79},
  // Skip G#
  // A
  {0b0000001110, 81},
  // A#
  {0b0001010110, 82},
  // B
  {0b0000000110, 83},
  // C
  {0b0000001010, 84},
};

int get_note() {
  // This routine reads the touch-sensitive keys of the instrument and
  // maps the value read to a MIDI note. We use a lookup table that maps
  // valid combinations of keys to a note. If the lookup fails, this
  // routine returns -1 to indicate that the fingering was not valid.
  int ret = -1;  // Sentinel for unknown fingering
  uint16_t touchValue = 0; // touchSensor.touched();
  // Since we're not using the 4th finger of the left hand, mask off that key
  touchValue = touchValue & 0b1111111111011111;

  for (uint8_t i = 0; i < FMAP_SIZE; i++) {
    if (touchValue == fmap[i].keys) {
      ret = fmap[i].midi_note;
      break;
    }
  }
  return ret;
}

int get_velocity(int initial, int final, unsigned long time_delta) {
  return map(final, NOTE_ON_THRESHOLD, MAX_PRESSURE, 0, 127);
}

void loop() {
  // read the breath sensor input on analog pin 0
  sensorValue = analogRead(A11);
  Serial.println(sensorValue);
//  
//  // It appears to be the case that if you don't read the sensor value
//  // periodically, it can get stuck. So read it here, and just throw away
//  // the data. We'll read it in get_note().
////  touchSensor.touched();  // Read sensor so it doesn't get stuck
//  if (state == NOTE_OFF) {
//    if (sensorValue > NOTE_ON_THRESHOLD) {
//      // Value has risen above threshold. Move to the RISE_TIME
//      // state. Record time and initial breath value.
//      breath_on_time = millis();
//      initial_breath_value = sensorValue;
//      state = RISE_WAIT;  // Go to next state
//    }
//  } else if (state == RISE_WAIT) {
//    if (sensorValue > NOTE_ON_THRESHOLD) {
//      // Has enough time passed for us to collect our second
//      // sample?
//      if (millis() - breath_on_time > RISE_TIME) {
//        // Yes, so calculate MIDI note and velocity, then send a note on event
//        noteSounding = get_note();
//        int velocity = get_velocity(initial_breath_value, sensorValue, RISE_TIME);
//        usbMIDI.sendNoteOn(noteSounding, velocity, MIDI_CHANNEL);
//        state = NOTE_ON;
//      }
//    } else {
//      // Value fell below threshold before RISE_TIME passed. Return to
//      // NOTE_OFF state (e.g. we're ignoring a short blip of breath)
//      state = NOTE_OFF;
//    }
//  } else if (state == NOTE_ON) {
//    if (sensorValue < NOTE_ON_THRESHOLD) {
//      // Value has fallen below threshold - turn the note off
//      usbMIDI.sendNoteOff(noteSounding, 100, MIDI_CHANNEL);  
//      state = NOTE_OFF;
//    } else {
//      // Is it time to send more aftertouch data?
//      if (millis() - atSendTime > AT_INTERVAL) {
//        // Map the sensor value to the aftertouch range 0-127
//        atVal = map(sensorValue, NOTE_ON_THRESHOLD, 1023, 0, 127);
//        usbMIDI.sendAfterTouch(atVal, MIDI_CHANNEL);
//        atSendTime = millis();
//      }
//    }
//    int newNote = get_note();
//    if (newNote == -1) {
//      return;  // Ignore unknown fingerings
//    }
//    if (newNote != noteSounding) {
//      // Player has moved to a new fingering while still blowing.
//      // Send a note off for the current node and a note on for
//      // the new note.
//      usbMIDI.sendNoteOff(noteSounding, 100, MIDI_CHANNEL);
//      noteSounding = newNote;
//      int velocity = get_velocity(initial_breath_value, sensorValue, RISE_TIME);
//      usbMIDI.sendNoteOn(noteSounding, velocity, MIDI_CHANNEL);
//    }
  }
//}
