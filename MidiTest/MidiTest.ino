void setup()
{
    //Set up serial output with standard MIDI baud rate
    Serial.begin(31250);
    
}

void loop()
{
    //Play a chromatic scale starting on middle C (60)
    for(int note = 60; note < 72; note++)
    {
        //Play a note
        playMIDINote(1, note, 100);
        //Hold note for 60 ms (delay() used for simplicity)
        delay(60);
        
        //Turn note off (velocity = 0)
        playMIDINote(1, note, 0);
        //Pause for 60 ms
        delay(60);
    }
}

void playMIDINote(byte channel, byte note, byte velocity)
{
    //MIDI channels 1-16 are really 0-15
    byte noteOnStatus=0x90 + (channel-1);  
    
    //Send notes to MIDI output:
    Serial.write(noteOnStatus);
    Serial.write(note);
    Serial.write(velocity);
}
 
