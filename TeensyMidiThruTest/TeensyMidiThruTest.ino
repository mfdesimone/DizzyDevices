#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

int chnl,d1,d2,dd;
void setup() {
  
  Serial.begin(31250);
  Serial.println("go");
}

void loop() {
  if (MIDI.read() &&  MIDI.getType() < midi::SystemExclusive) {
    Serial.println("dude");
    d1 = MIDI.getData1();
    d2 = MIDI.getData2();
    dd = d1 + (d2 << 8);
    chnl = MIDI.getChannel();
    // and then send...
    switch(MIDI.getType()){
      case midi::NoteOn:
        usbMIDI.sendNoteOn(d1,d2,chnl);
        Serial.println(d1);
        Serial.println(d2);
      break;
      case midi::NoteOff:
        usbMIDI.sendNoteOff(d1,d2,chnl);
      break;
      case midi::AfterTouchPoly:
        usbMIDI.sendPolyPressure(d1,d2,chnl);
      break;
      case midi::ControlChange:
        usbMIDI.sendControlChange(d1,d2,chnl);
      break;
      case midi::ProgramChange:
        usbMIDI.sendProgramChange(dd,chnl);
      break;
      case midi::AfterTouchChannel:
        usbMIDI.sendAfterTouch(dd,chnl);
      break;
      case midi::PitchBend:
        
        usbMIDI.sendPitchBend(dd,chnl);
      break;
      case midi::SystemExclusive:
        // handle sysex
      break;
      default:
        // F8 et seq.
      break;
    }
  }
  while (usbMIDI.read()) {
    // ignore incoming messages
  }
//  if (usbMIDI.read() &&  usbMIDI.getType() < SystemExclusive) {
//    type = (kMIDIType) usbMIDI.getType();
//    d1 = usbMIDI.getData1();
//    d2 = usbMIDI.getData2();
//    
//    chnl = usbMIDI.getChannel();
//    // and then send...
//    MIDI.send(type,d1,d2,chnl);
//  }
}
