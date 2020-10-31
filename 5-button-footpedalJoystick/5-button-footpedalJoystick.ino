/* Complete USB Joystick Example
   Teensy becomes a USB joystick with 16 or 32 buttons and 6 axis input

   You must select Joystick from the "Tools > USB Type" menu

   Pushbuttons should be connected between the digital pins and ground.
   Potentiometers should be connected to analog inputs 0 to 5.

   This example code is in the public domain.
*/

// Configure the number of buttons.  Be careful not
// to use a pin for both a digital button and analog
// axis.  The pullup resistor will interfere with
// the analog voltage.
const int numButtons = 5;  // 16 for Teensy, 32 for Teensy++

void setup() {
  // you can print to the serial monitor while the joystick is active!
  Serial.begin(9600);
  // configure the joystick to manual send mode.  This gives precise
  // control over when the computer receives updates, but it does
  // require you to manually call Joystick.send_now().
  Joystick.useManualSend(true);
  for (int i=0; i<numButtons; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  Serial.println("Begin Complete Joystick Test");
}

byte allButtons[numButtons];
byte prevButtons[numButtons];
int angle=0;
int joyX=0;
int joyY=0;
int lowThrottle = 0;
int medThrottle = 0;
int highThrottle = 0;
int fullThrottle = 0;
int revMedThrottle = 0;
int revFullThrottle = 0;

void loop() {
  // read 6 analog inputs and use them for the joystick axis
  

//  Joystick.Z(analogRead(2));
//  Joystick.Zrotate(analogRead(3));
//  Joystick.sliderLeft(analogRead(4));
//  Joystick.sliderRight(analogRead(5));
  
  // read digital pins and use them for the buttons
  for (int i=0; i<numButtons; i++) {
    if (digitalRead(i)) {
      // when a pin reads high, the button is not pressed
      // the pullup resistor creates the "on" signal
      switch(i){
        case 3:
          revMedThrottle = 0;
          break;
        case 4:
          revFullThrottle = 0;
          break;
        case 2:
          lowThrottle = 0;
          break;
        case 0:
          medThrottle = 0;
          break;
        case 1:
          fullThrottle = 0;
          break;
        default:
        break;
      }
      allButtons[i] = 0;
    } else {
      // when a pin reads low, the button is connecting to ground.
      allButtons[i] = 1;
      switch(i){
        case 3:
          revMedThrottle = 300;
          break;
        case 4:
          revFullThrottle = 723;
          break;
        case 0:
          lowThrottle = 100;
          break;
        case 1:
          medThrottle = 200;
          break;
        case 2:
          fullThrottle = 723;
          break;
        default:
        break;
      }
    }
    joyX = revFullThrottle + revMedThrottle;
    joyY = lowThrottle + medThrottle + fullThrottle;
    Joystick.X(joyX);
    Joystick.Y(joyY);
    if(joyX){
    Serial.print("joyX: ");
    Serial.print(joyX);
    Serial.println();
    }
    if(joyY){
    Serial.print("joyY: ");
    Serial.print(joyY);
    Serial.println();
    }
//    Joystick.button(i + 1, allButtons[i]);
  }

//  // make the hat switch automatically move in a circle
//  angle = angle + 1;
//  if (angle >= 360) angle = 0;
//  Joystick.hat(angle);
//  
  // Because setup configured the Joystick manual send,
  // the computer does not see any of the changes yet.
  // This send_now() transmits everything all at once.
  Joystick.send_now();
  
  // check to see if any button changed since last time
  boolean anyChange = false;
  for (int i=0; i<numButtons; i++) {
    if (allButtons[i] != prevButtons[i]) anyChange = true;
    prevButtons[i] = allButtons[i];
  }
  
  // if any button changed, print them to the serial monitor
  if (anyChange) {
    lowThrottle = 0;
    medThrottle = 0;
    highThrottle = 0;
    fullThrottle = 0;
    revMedThrottle = 0;
    revFullThrottle = 0;
//    Serial.print("Buttons: ");
//    for (int i=0; i<numButtons; i++) {
//      Serial.print(allButtons[i], DEC);
//    }
//    Serial.println();
  }
  
  // a brief delay, so this runs "only" 200 times per second
  delay(5);
}
