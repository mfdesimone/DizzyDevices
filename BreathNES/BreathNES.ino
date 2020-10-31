
#define touch_Thr 2000
#define delayAmount 20
// The LED pins for the four d-pad directions
const int E = 2;
const int S = 3;
const int W = 4;  
const int N = 5; 
//const int ledPin =  13;      // the number of the LED pin
const int numButtons = 8;  
int breath = 870; // initial value for breath, in the deadzone  of 850-880
int breathState = 0;

void setup() {
  // you can print to the serial monitor while the joystick is active!
  pinMode(E, OUTPUT);
  pinMode(S, OUTPUT);
  pinMode(W, OUTPUT);
  pinMode(N, OUTPUT);
  Serial.begin(9600);
  // configure the joystick to manual send mode.  This gives precise
  // control over when the computer receives updates, but it does
  // require you to manually call Joystick.send_now().
//  Joystick.useManualSend(true);
  Serial.println("Begin Complete Joystick Test");
    Joystick.X(512);
    Joystick.Y(512);
}
byte allButtons[numButtons];
byte prevButtons[numButtons];

void padRead(){
  // read digital pins and use them for the buttons
  for (int i=0; i<4; i++) {
    int reading = touchRead(i+16);
    if ( reading > touch_Thr) {
      allButtons[i] = 1;
      Serial.println("button");
      Serial.println(i);
    } else {
      allButtons[i] = 0;
    }
    Joystick.button(i + 1, allButtons[i]);
  }
}

void breathMap(){
  int breathReading = breath / 8;
//    Serial.println(breathReading);
  breathState = 0; //no direction
  
  if (breathReading > 100 && breathReading < 107){
    breathState = 1; // E
    Serial.println("E");
  } else
  if (breathReading > 96 && breathReading < 101){
    breathState = 2; // SE
    Serial.println("SE");
  } else
  if (breathReading > 90 && breathReading < 97){
    breathState = 3; // S
    Serial.println("S");
  } else
  if (breathReading > 86 && breathReading < 91){
    breathState = 4; // SW
    Serial.println("SW");
  } else
  if (breathReading > 80 && breathReading < 87){
    breathState = 5; // W
    Serial.println("W");
  } else
  if (breathReading > 75 && breathReading < 81){
    breathState = 6; // NW
    Serial.println("NW");
  } else
  if ( breathReading < 76){ //breathReading > 60 &&
    breathState = 7; // N
    Serial.println("N");
  } else
  
  if (breathReading > 109 && breathReading < 116) {
    breathState = 5; // W
    Serial.println("W");
  }  else
  if (breathReading > 115 && breathReading < 120){
    breathState = 6; // NW
    Serial.println("NW");
  } else
  if (breathReading > 119 && breathReading < 124){
    breathState = 7; // N
    Serial.println("N");
  } else
  if (breathReading > 123){
    breathState = 8; // NE
    Serial.println("NE");
  }

//  Serial.println(breathState);
    
  
  switch (breathState) {
  case 0: //no direction
    digitalWrite(E, LOW);
    digitalWrite(S, LOW);
    digitalWrite(W, LOW);
    digitalWrite(N, LOW);
//    Joystick.button(5,0);
//    Joystick.button(6,0);
//    Joystick.button(7,0);
//    Joystick.button(8,0);
    Joystick.X(512);
    Joystick.Y(512);
    break;
  case 1: // E
    digitalWrite(E, HIGH);
    digitalWrite(S, LOW);
    digitalWrite(W, LOW);
    digitalWrite(N, LOW);
    Joystick.X(1023);
    Joystick.Y(512);    
//    Joystick.button(5,1);
//    Joystick.button(6,0);
//    Joystick.button(7,0);
//    Joystick.button(8,0);
    break;
  case 2: // SE
    digitalWrite(E, HIGH);
    digitalWrite(S, HIGH);
    digitalWrite(W, LOW);
    digitalWrite(N, LOW);
//    Joystick.button(5,1);
//    Joystick.button(6,1);
//    Joystick.button(7,0);
//    Joystick.button(8,0);
    Joystick.X(1023);
    Joystick.Y(0);
    break;
  case 3: // S
    digitalWrite(E, LOW);
    digitalWrite(S, HIGH);
    digitalWrite(W, LOW);
    digitalWrite(N, LOW);
//    Joystick.button(5,0);
//    Joystick.button(6,1);
//    Joystick.button(7,0);
//    Joystick.button(8,0);
    Joystick.X(512);
    Joystick.Y(0);
    break;
  case 4: // SW
    digitalWrite(E, LOW);
    digitalWrite(S, HIGH);
    digitalWrite(W, HIGH);
    digitalWrite(N, LOW);
//    Joystick.button(5,0);
//    Joystick.button(6,1);
//    Joystick.button(7,1);
//    Joystick.button(8,0);
    Joystick.X(0);
    Joystick.Y(0);
    break; 
  case 5: // W
    digitalWrite(E, LOW);
    digitalWrite(S, LOW);
    digitalWrite(W, HIGH);
    digitalWrite(N, LOW);
//    Joystick.button(5,0);
//    Joystick.button(6,0);
//    Joystick.button(7,1);
//    Joystick.button(8,0);
    Joystick.X(0);
    Joystick.Y(512);
    break;
  case 6: // NW
    digitalWrite(E, LOW);
    digitalWrite(S, LOW);
    digitalWrite(W, HIGH);
    digitalWrite(N, HIGH);
//    Joystick.button(5,0);
//    Joystick.button(6,0);
//    Joystick.button(7,1);
//    Joystick.button(8,1);
    Joystick.X(0);
    Joystick.Y(1024);
    break;
  case 7: // N
    digitalWrite(E, LOW);
    digitalWrite(S, LOW);
    digitalWrite(W, LOW);
    digitalWrite(N, HIGH);
//    Joystick.button(5,0);
//    Joystick.button(6,0);
//    Joystick.button(7,0);
//    Joystick.button(8,1);
    Joystick.X(512);
    Joystick.Y(1023);
    break;
  case 8: // NE
    digitalWrite(E, HIGH);
    digitalWrite(S, LOW);
    digitalWrite(W, LOW);
    digitalWrite(N, HIGH);
//    Joystick.button(5,1);
//    Joystick.button(6,0);
//    Joystick.button(7,0);
//    Joystick.button(8,1);
    Joystick.X(1023);
    Joystick.Y(1023);
    break;
    
  default: //no direction
    digitalWrite(E, LOW);
    digitalWrite(S, LOW);
    digitalWrite(W, LOW);
    digitalWrite(N, LOW);
//    Joystick.button(5,0);
//    Joystick.button(6,0);
//    Joystick.button(7,0);
//    Joystick.button(8,0);
    Joystick.X(512);
    Joystick.Y(512);
    break;
  }
}

void loop() {
  breath = analogRead(A6);
  breathMap();
  padRead();
//  Joystick.send_now();
  delay(delayAmount);
  
}
