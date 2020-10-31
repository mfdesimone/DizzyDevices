// -------------------------------------------------------------
// Arduino control of NeoPixel strips via MIDI.

#include <MIDI.h>               // Use1ByteParsing set to false, and BaudRate set to 115200
#include <Adafruit_NeoPixel.h>
#include <HardwareSerial.h>     // Increased the buffer size to 1024

#define PIN 6

#define NUM_PIXELS  12  

#define CHANNEL_RED   1
#define CHANNEL_GREEN 2
#define CHANNEL_BLUE  3
//#define CHANNEL_WHITE 4

#define CC_BRIGHTNESS 74

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

MIDI_CREATE_DEFAULT_INSTANCE();  // Baud rate needs to be changed to 115200 in midi_Settings.h !!!!   Hairless MIDI must match

boolean changed = false;

// ----------------------------------------------------------------------------

void setup()
{
    MIDIsetup();
    strip.begin();
    PixelRefresh();
}

void loop()
{
    // Don't do a heck of a lot here.  All the fun stuff is done in the MIDI handlers.
    changed = false;

    // Call MIDI.read the fastest you can for real-time performance.
    MIDI.read();

    if (changed)
    {
        PixelRefresh();
        changed = false;
    }
}

// ----------------------------------------------------------------------------

void MIDIsetup()
{
    // Initiate MIDI communications, listen to all channels
    MIDI.begin(MIDI_CHANNEL_OMNI);

    // No MIDIThru
    MIDI.turnThruOff();

    // Connect the functions to the library
    MIDI.setHandleNoteOn(HandleNoteOn);
    MIDI.setHandleNoteOff(HandleNoteOff);
    MIDI.setHandleControlChange(HandleControlChange);
}

inline void PixelRefresh()
{
    strip.show();
}

// -----------------------------------------------------------------------------
void HandleNoteOn(byte channel, byte note, byte velocity)
{
    // This acts like a NoteOff.
    if (velocity == 0)
    {
        HandleNoteOff(channel, note, velocity);
        return;
    }

    byte brightness = velocity << 2;  // 0 to 254

    uint32_t current_color = strip.getPixelColor(note);

    byte red = (current_color >> 16) & 0xFF;
    byte green = (current_color >> 8) & 0xFF;
    byte blue = (current_color)& 0xFF;

    switch (channel)
    {
    case CHANNEL_RED:
        red = brightness;
        break;

    case CHANNEL_GREEN:
        green = brightness;
        break;

    case CHANNEL_BLUE:
        blue = brightness;
        break;

//    case CHANNEL_WHITE:
//        red = brightness;
//        green = brightness;
//        blue = brightness;
//        break;

    default:  // Ignore all others
        return;
    }

    strip.setPixelColor(note, red, green, blue);
    changed = true;
}

// -----------------------------------------------------------------------------
void HandleNoteOff(byte channel, byte note, byte velocity)
{
    uint32_t current_color = strip.getPixelColor(note);

    byte red = (current_color >> 16) & 0xFF;
    byte green = (current_color >> 8) & 0xFF;
    byte blue = (current_color)& 0xFF;

    switch (channel)
    {
    case CHANNEL_RED:
        red = 0;
        break;

    case CHANNEL_GREEN:
        green = 0;
        break;

    case CHANNEL_BLUE:
        blue = 0;
        break;

//    case CHANNEL_WHITE:
//        red = 0;
//        green = 0;
//        blue = 0;
//        break;

    default:  // Ignore all others
        return;
    }

    strip.setPixelColor(note, red, green, blue);
    changed = true;
}


void HandleControlChange(byte channel, byte number, byte value)
{
    switch (number)
    {
    case CC_BRIGHTNESS:
        HandleBrightness(channel, value * 2);
        break;

    default:  // Ignore all others
        return;
    }
}

uint32_t HandleBrightness(byte channel, byte brightness)
{
    byte red = 0;
    byte green = 0;
    byte blue = 0;

    switch (channel)
    {
    case CHANNEL_RED:
        red = brightness;
        break;

    case CHANNEL_GREEN:
        green = brightness;
        break;

    case CHANNEL_BLUE:
        blue = brightness;
        break;

//    case CHANNEL_WHITE:
//        red = brightness;
//        green = brightness;
//        blue = brightness;
//        break;

    default:  // Ignore all others
        return 0;
    }

    for (int i = 0; i < NUM_PIXELS; i++)
    {
        strip.setPixelColor(i, red, green, blue);
    }
    changed = true;
}

// EOF
