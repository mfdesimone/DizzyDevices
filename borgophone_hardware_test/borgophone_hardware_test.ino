#include <Bounce2.h>

// *************** pixel setup ****************
#include "FastLED.h" //the FastLED library that drives all the LED data formatting. This project assumes you're using a strand of neoPixels
#define NUM_LEDS 12 // How many leds in your strip?
#define BRIGHTNESS  24
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 100

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;


CRGB leds[NUM_LEDS]; // Define the array of leds

volatile byte state = LOW; //for interrupt

// ***************Pin definitions***************

// TOUCHPADS
#define touch_Thr 1300  // threshold for Teensy touchRead, 1300-1800
#define RH1pin 0
#define RH2pin 1
#define RH3pin 22
#define RH4pin 23
#define LH1pin 15
#define LH2pin 16
#define LH3pin 17
#define LH4pin 18
#define TH1pin 19
#define TH2pin 25
#define TH3pin 32
#define TH4pin 33

            // Key variables, TRUE (1) for pressed, FALSE (0) for not pressed
byte LH1;   
byte LH2;  
byte LH3;   
byte LH4;  
byte RH1;   
byte RH2;  
byte RH3;   
byte RH4;  
byte TH1;
byte TH2;
byte TH3;
byte TH4;

// BUTTONS
#define octUppin 26 // button to transpose instrument an octave up
#define octDnpin 27 // button to transpose instrument an octave down
#define chUppin 4
#define chDnpin 5
#define progChUppin 6
#define progChDnpin 7
#define modeSelpin 11
#define curveSelpin 12

// BUTTON DEBOUNCING
int debounceInterval = 5;
bool OctUpState = 0;
bool OctDnState = 0;
bool ProgChgupState = 0;
bool ProgChgDnState = 0;
bool MidChUpState = 0;
bool MidChDnState = 0;
Bounce OctUpDebo = Bounce();// Instantiate a Bounce object
Bounce OctDnDebo = Bounce();
Bounce ProgChgupDebo = Bounce();
Bounce ProgChgDnDebo = Bounce();
Bounce MidChUpDebo = Bounce();
Bounce MidChDnDebo = Bounce();
Bounce ModeDebo = Bounce();
Bounce CurvePinDebo = Bounce();
Bounce OctADebo = Bounce();
Bounce OctBDebo = Bounce();


// SWITCHES
#define ccSwitchpin 24
#define OctApin 2 // limit switch for momentary octave up
#define OctBpin 3 // limit switch for momentary octave down

// LEDS
#define pixelPin 8
#define faceLEDpin 9
#define frontLEDpin 10
#define onboardLEDpin 13

// POTENTIOMETERS / ANALOG
#define joyXpin 21
#define joyYpin 20
#define pot1pin 28
#define pot2pin 31
#define pot3pin A10
#define pot4pin A11
#define breathpin A12
#define fsrpin A13
int joyXReading = 0;
int joyYReading = 0;
int pot1Reading = 0;
int pot2Reading = 0;
int pot3Reading = 0;
int pot4Reading = 0;
int breathReading = 0;
int fsrReading = 0;

// BUTTON STATES
int octUpState = 0; 
int octDnState = 0; 
int chUpState = 0; 
int chDnState = 0; 
int progChUpState = 0; 
int progChDnState = 0;
int modeSelState = 0; 
int curveSelState = 0; 
int ccSwitchState = 0;
int OctAState = 0;
int OctBState = 0;


// TEST VARIABLES
unsigned long instrumentModeCounter = 0;
unsigned long curveCounter = 0;
int numberOfCurves = 13;
int numberOfModes = 8;
int instrumentMode = 0;
int octave = 0; 
int ProgChangeNumber = 0; 
byte MIDIchannel=1;
int curve = 0;

void setup() {
  // ***************Pin setups***************
  
  pinMode(octUppin, INPUT_PULLUP);
  pinMode(octDnpin, INPUT_PULLUP); 
  pinMode(chUppin, INPUT_PULLUP); 
  pinMode(chDnpin, INPUT_PULLUP); 
  pinMode(progChUppin, INPUT_PULLUP); 
  pinMode(progChDnpin, INPUT_PULLUP); 
  pinMode(modeSelpin, INPUT_PULLUP); 
  pinMode(curveSelpin, INPUT_PULLUP);
  pinMode(ccSwitchpin, INPUT_PULLUP);
  pinMode(OctApin, INPUT_PULLUP);
  pinMode(OctBpin, INPUT_PULLUP);
  pinMode(faceLEDpin, OUTPUT); 
  pinMode(frontLEDpin, OUTPUT);  
  pinMode(onboardLEDpin, OUTPUT); 
      Serial.begin(115200); // Go serial go!
      
  // *************** pixel setup ***************
  delay( 1000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, pixelPin, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  // *************** button debounce setups ***************
  OctUpDebo.attach(octUppin); // After setting up the button, setup the Bounce instance
  OctUpDebo.interval(debounceInterval); // interval in ms
  OctDnDebo.attach(octDnpin);
  OctDnDebo.interval(debounceInterval); // interval in ms
  ProgChgupDebo.attach(progChUppin);
  ProgChgupDebo.interval(debounceInterval); // interval in ms
  ProgChgDnDebo.attach(progChDnpin);
  ProgChgDnDebo.interval(debounceInterval); // interval in ms
  MidChUpDebo.attach(chUppin);
  MidChUpDebo.interval(debounceInterval); // interval in ms
  MidChDnDebo.attach(chDnpin);
  MidChDnDebo.interval(debounceInterval); // interval in ms
  ModeDebo.attach(modeSelpin);
  ModeDebo.interval(debounceInterval); // interval in ms
  CurvePinDebo.attach(curveSelpin);
  CurvePinDebo.interval(debounceInterval); // interval in ms
  OctADebo.attach(OctApin);
  OctADebo.interval(debounceInterval);
  OctBDebo.attach(OctBpin);
  OctBDebo.interval(debounceInterval);
}

void loop(){
  if (digitalRead(ccSwitchpin) == HIGH){
    ccSwitchState = 1;
  } else if (digitalRead(ccSwitchpin) == LOW){ 
    ccSwitchState = 0;
  }
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  FillLEDsFromPaletteColors( startIndex);
  ChangePalettePeriodically();
  FastLED.show();
  FastLED.delay(1000 / UPDATES_PER_SECOND);

  if (ccSwitchState == 1){
    digitalWrite(frontLEDpin, HIGH);
    digitalWrite(faceLEDpin, HIGH);
  } else {
    digitalWrite(frontLEDpin, LOW);
    digitalWrite(faceLEDpin, LOW);
  }
  testTouchPads();
  testButtons();
//  Serial.println(analogRead(joyXpin));
//  Serial.println(analogRead(joyYpin));
//  Serial.println(analogRead(breathpin));
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}

void testTouchPads(){
  RH1 = touchRead(RH1pin)>touch_Thr;
  RH2 = touchRead(RH2pin)>touch_Thr;
  RH3= touchRead(RH3pin)>touch_Thr;
  RH4= touchRead(RH4pin)>touch_Thr;
  LH1= touchRead(LH1pin)>touch_Thr;
  LH2= touchRead(LH2pin)>touch_Thr;
  LH3= touchRead(LH3pin)>touch_Thr;
  LH4= touchRead(LH4pin)>touch_Thr;
  TH1= touchRead(TH1pin)>touch_Thr;
  TH2= touchRead(TH2pin)>touch_Thr;
  TH3= touchRead(TH3pin)>touch_Thr;
  TH4= touchRead(TH4pin)>touch_Thr;
  if(RH1){Serial.println("RH1");}
  if(RH2){Serial.println("RH2");}
  if(RH3){Serial.println("RH3");}
  if(RH4){Serial.println("RH4");}
  if(LH1){Serial.println("LH1");}
  if(LH2){Serial.println("LH2");}
  if(LH3){Serial.println("LH3");}
  if(LH4){Serial.println("LH4");}
  if(TH1){Serial.println("TH1");}
  if(TH2){Serial.println("TH2");}
  if(TH3){Serial.println("TH3");}
  if(TH4){Serial.println("TH4");}
}

void testButtons(){
  // Update the Bounce instances :
  testPots();
  OctUpDebo.update();
  OctDnDebo.update();
  ProgChgupDebo.update();
  ProgChgDnDebo.update();
  MidChUpDebo.update();
  MidChDnDebo.update();
  ModeDebo.update();
  CurvePinDebo.update();
  OctADebo.update();
  OctBDebo.update();

  if (OctUpDebo.fell()){
      octave ++;  // octave up
      Serial.println("Transposed Octave Up");
      Serial.println(octave);
      Serial.println("joyX");
      Serial.println(joyXReading);
    }
    if (OctDnDebo.fell()){
      octave --;  // octave down
      Serial.println("Transposed Octave Down");
      Serial.println(octave);
      Serial.println("joyY");
      Serial.println(joyYReading);
    }
    if (ProgChgupDebo.fell() && ProgChangeNumber < 127 ){
      ProgChangeNumber ++;  // transpose up half step
      Serial.println("Program Change");
      Serial.println(ProgChangeNumber);
      Serial.println("breath");
      Serial.println(breathReading);
    }
    if (ProgChgDnDebo.fell() && ProgChangeNumber > 0 ){
      ProgChangeNumber --;  // transpose down half step
      Serial.println("Program Change");
      Serial.println(ProgChangeNumber);
      Serial.println("fsr");
      Serial.println(fsrReading);
     }
    if (MidChUpDebo.fell()&& MIDIchannel < 16 ){
      MIDIchannel ++;
      Serial.println("Midi +");
      Serial.println(MIDIchannel);
      Serial.println("pot1");
      Serial.println(pot1Reading);
    }
    if (MidChDnDebo.fell()&& MIDIchannel > 1 ){ //using Midi channel down for mode selection at the moment
      MIDIchannel --;
      Serial.println("Midi -");
      Serial.println(MIDIchannel);
      Serial.println("pot2");
      Serial.println(pot2Reading);
    }
    if (ModeDebo.fell() ){
      if (instrumentModeCounter < numberOfModes){
        instrumentModeCounter++;
      } else {
        instrumentModeCounter = 0;
      }
      modeSelector();
      Serial.println("Mode");
      Serial.println(instrumentModeCounter);
      Serial.println("pot3");
      Serial.println(pot3Reading);
    }
    if (CurvePinDebo.fell() ){
      curveCounter++;
      curveSelector();
      Serial.println("curve");
      Serial.println(curve);
      Serial.println("pot4");
      Serial.println(pot4Reading);
    }
    if (OctADebo.fell() ){
      Serial.println("8va");
    }
    if (OctBDebo.fell() ){
      Serial.println("8vb");
    }
}
void curveSelector(){
  curve = curveCounter % numberOfCurves;
}
void modeSelector(){
  instrumentMode = instrumentModeCounter % numberOfModes;
}
void testPots(){
  joyXReading = analogRead(joyXpin);
  joyYReading = analogRead(joyYpin);
  breathReading = analogRead(breathpin);
  fsrReading = analogRead(fsrpin);
  pot1Reading = analogRead(pot1pin);
  pot2Reading = analogRead(pot2pin);
  pot3Reading = analogRead(pot3pin);
  pot4Reading = analogRead(pot4pin);
}
// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;
    
    if( lastSecond != secondHand) {
        lastSecond = secondHand;
        if( secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
        if( secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
        if( secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
        if( secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
        if( secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
        if( secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
        if( secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
        if( secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
        if( secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
        if( secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
    }
}

// This function fills the palette with totally random colors.
void SetupBlackPalette(){
  for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( 0, 0, 0);
    }
}

void SetupTotallyRandomPalette()
{
    for( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;
    
    currentPalette = CRGBPalette16(
                                   green,  green,  black,  black,
                                   purple, purple, black,  black,
                                   green,  green,  black,  black,
                                   purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};



// Additional notes on FastLED compact palettes:
//
// Normally, in computer graphics, the palette (or "color lookup table")
// has 256 entries, each containing a specific 24-bit RGB color.  You can then
// index into the color palette using a simple 8-bit (one byte) value.
// A 256-entry color palette takes up 768 bytes of RAM, which on Arduino
// is quite possibly "too many" bytes.
//
// FastLED does offer traditional 256-element palettes, for setups that
// can afford the 768-byte cost in RAM.
//
// However, FastLED also offers a compact alternative.  FastLED offers
// palettes that store 16 distinct entries, but can be accessed AS IF
// they actually have 256 entries; this is accomplished by interpolating
// between the 16 explicit entries to create fifteen intermediate palette
// entries between each pair.
//
// So for example, if you set the first two explicit entries of a compact 
// palette to Green (0,255,0) and Blue (0,0,255), and then retrieved 
// the first sixteen entries from the virtual palette (of 256), you'd get
// Green, followed by a smooth gradient from green-to-blue, and then Blue.
