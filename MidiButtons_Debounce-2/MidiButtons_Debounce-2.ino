/* 
 DESCRIPTION
 ====================
 Simple example of the Bounce library that switches the debug LED when 
 either of 2 buttons are pressed.
 */
 
// Include the Bounce2 library found here :
// https://github.com/thomasfredericks/Bounce2
#include <Bounce2.h>

// Let's use #define to name the buttons and link them to their input pins on the Arduino
#define b_0 0
#define b_1 2
#define b_2 4
#define b_3 3
#define b_4 5
#define b_5 7
#define b_6 8
#define b_7 6
#define b_8 10
#define b_9 9
#define b_10 A2
#define b_11 12
#define b_12 11
#define b_func A3
#define b_page 1
#define LED1 A5
#define LED2 A4
#define ledPin 13      // the number of the onboard LED pin

// Let's define some more global variables for the program
char deviceMode[4] = "note"; // this string variable will keep track of which mode the device is in. Options should include note, prog, chan, and note should be default.
// note mode: buttons 0-12 will output midi notes 0-12 + 12 * the page number, allowing for multiple octaves
// chan mode: buttons 0-9 will select the midi channel, one digit at a time (press 0, 1 for channel 1, for example)
// prog mode: buttons will send out a program change message, using numbers and the page selector if needed
bool page = false; //  if true, the page selection mode is active. 
char buttonReadings[8]; 
int octaveOffset = 0; // track which octave we're in. 

// Instantiate Bounce objects - one for each button
Bounce debouncerb_0 = Bounce(); 
Bounce debouncerb_1 = Bounce(); 
Bounce debouncerb_2 = Bounce(); 
Bounce debouncerb_3 = Bounce(); 
Bounce debouncerb_4 = Bounce(); 
Bounce debouncerb_5 = Bounce(); 
Bounce debouncerb_6 = Bounce(); 
Bounce debouncerb_7 = Bounce(); 
Bounce debouncerb_8 = Bounce(); 
Bounce debouncerb_9 = Bounce(); 
Bounce debouncerb_10 = Bounce(); 
Bounce debouncerb_11 = Bounce(); 
Bounce debouncerb_12 = Bounce(); 
Bounce debouncerb_func = Bounce(); 
Bounce debouncerb_page = Bounce(); 



void setup() {

  //start serial connection
  Serial.begin(9600);
  // Set the pin modees on each of pins 
  pinMode(b_0, INPUT_PULLUP);
  pinMode(b_1, INPUT_PULLUP);
  pinMode(b_2, INPUT_PULLUP);  
  pinMode(b_3, INPUT_PULLUP);
  pinMode(b_4, INPUT_PULLUP);
  pinMode(b_5, INPUT_PULLUP);
  pinMode(b_6, INPUT_PULLUP);
  pinMode(b_7, INPUT_PULLUP);
  pinMode(b_8, INPUT_PULLUP);
  pinMode(b_9, INPUT_PULLUP);
  pinMode(b_10, INPUT_PULLUP);
  pinMode(b_11, INPUT_PULLUP);
  pinMode(b_12, INPUT_PULLUP);
  pinMode(b_func, INPUT_PULLUP);
  pinMode(b_page, INPUT_PULLUP);
  //  pinMode(A5, OUTPUT); //LED 1
  //  pinMode(A4, OUTPUT); //LED 2
  pinMode(ledPin, OUTPUT); 
  
  
  // After setting up the buttons, setup the Bounce instance :
  debouncerb_0.attach(b_0);
  debouncerb_1.attach(b_1);
  debouncerb_2.attach(b_2);
  debouncerb_3.attach(b_3);
  debouncerb_4.attach(b_4);
  debouncerb_5.attach(b_5);
  debouncerb_6.attach(b_6);
  debouncerb_7.attach(b_7);
  debouncerb_8.attach(b_8);
  debouncerb_9.attach(b_9);
  debouncerb_10.attach(b_10);
  debouncerb_11.attach(b_11);
  debouncerb_12.attach(b_12);
  debouncerb_func.attach(b_func);
  debouncerb_page.attach(b_page);
  // define the debounce wait period (in milliseconds)
  debouncerb_0.interval(5);
  debouncerb_1.interval(5);
  debouncerb_2.interval(5);
  debouncerb_3.interval(5);
  debouncerb_4.interval(5);
  debouncerb_5.interval(5);
  debouncerb_6.interval(5);
  debouncerb_7.interval(5);
  debouncerb_8.interval(5);
  debouncerb_9.interval(5);
  debouncerb_10.interval(5);
  debouncerb_11.interval(5);
  debouncerb_12.interval(5);
  debouncerb_func.interval(5);
  debouncerb_page.interval(5);

  //Setup the LED :
  pinMode(ledPin,OUTPUT);
}

// main program loop
void loop() {
  //int buttonReadings = readButtons(); // read the buttons
  Serial.println(octaveOffset);
  Serial.println(buttonReadings);
  Serial.println(page);

// Update the Bounce instances :
  debouncerb_0.update();
  debouncerb_1.update();
  debouncerb_2.update();
  debouncerb_3.update();
  debouncerb_4.update();
  debouncerb_5.update();
  debouncerb_6.update();
  debouncerb_7.update();
  debouncerb_8.update();
  debouncerb_9.update();
  debouncerb_10.update();
  debouncerb_11.update();;
  debouncerb_12.update();
  debouncerb_func.update();
  debouncerb_page.update();

  // Get the updated value :
  int value0 = debouncerb_0.read();
  int value1 = debouncerb_1.read();
  int value2 = debouncerb_2.read();
  int value3 = debouncerb_3.read();
  int value4 = debouncerb_4.read();
  int value5 = debouncerb_5.read();
  int value6 = debouncerb_6.read();
  int value7 = debouncerb_7.read();
  int value8 = debouncerb_8.read();
  int value9 = debouncerb_9.read();
  int value10 = debouncerb_10.read();
  int value11 = debouncerb_11.read();
  int value12 = debouncerb_12.read();  
  int valuepage = debouncerb_func.read();
  int valuefunc = debouncerb_page.read();

  // Let's set up the logic behind the buttons. 
    // if the page button was pressed and we're not in page mode
  if ( valuepage == 1 && !page ){ 
    page = true;
    
    // if the page button was pressed and we're already in page mode, let's turn off page mode
    } else if ( valuepage == 1 && page ){ 
     page = false;
    }
    
     // if button b_0 was pressed
    if ( value0 == 1 ){
      if ( !page ) {  
        return (0 + 12 * octaveOffset); // return a zero, plus however many half steps are in the octave offset
      } else if ( page ) {
        octaveOffset = 0;
      }
    }
    if ( value1 == 1 ){
      if ( !page ) {  
        return (1 + 12 * octaveOffset);
      } else if ( page) {
        octaveOffset = 1;
      }
    }
    if ( value2 == 1 ){ 
      if ( !page ) {  
        return (2 + 12 * octaveOffset);
        } else if ( page) {
        octaveOffset = 2;
      }
    }
    if ( value3 == 1 ){ 
      if ( !page ) {  
        return (3 + 12 * octaveOffset);
      } else if ( page) {
        octaveOffset = 3;
      }
    }
    if ( value4 == 1 ){ 
      if ( !page ) {  
        return (4 + 12 * octaveOffset);
      } else if ( page) {
        octaveOffset = 4;
      }
    } 
    if ( value5 == 1 ){ 
      if ( !page ) {  
      return (5 + 12 * octaveOffset);
      } else if ( page) {
        octaveOffset = 4;
      }
    }
    if ( value6 == 1 ){ 
      if ( !page ) {  
        return (6 + 12 * octaveOffset);
      } else if ( page) {
        octaveOffset = 4;
      }
    }
    if ( value7 == 1 ){ 
      if ( !page ) {  
        return (7 + 12 * octaveOffset);
      } else if ( page) {
        octaveOffset = 4;
      }
    }
    if ( value8 == 1 ){ 
      if ( !page ) {  
        return (8 + 12 * octaveOffset);
      } else if ( page) {
        octaveOffset = 4;
      }
    }
    if ( value9 == 1 ){ 
      if ( !page ) {  
        return (9 + 12 * octaveOffset);
      } else if ( page) {
        octaveOffset = 4;
      }
    }
    if ( value10 == 1 ){ 
      if ( !page ) {  
        return (10 + 12 * octaveOffset);
      } else if ( page) {
        octaveOffset = 4;
      }
    }
    if ( value11 == 1 ){ 
      if ( !page ) {  
        return (11 + 12 * octaveOffset);
      } else if ( page) {
        octaveOffset = 4;
      }
    }
    if ( value12 == 1 ){ 
      if ( !page ) {  
        return (12 + 12 * octaveOffset);
      } else if ( page) {
        octaveOffset = 4;
      }
      } 
 
    
    if ( valuefunc == 1 ){ 
    
    }
   else {
    return;
   }
    





  
  delay(200);
}
