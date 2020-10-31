/* Buttons to USB Keyboard Example

   You must select Keyboard from the "Tools > USB Type" menu

   This example code is in the public domain.
*/

#include <Bounce.h>

// Create Bounce objects for each button.  The Bounce object
// automatically deals with contact chatter or "bounce", and
// it makes detecting changes very simple.
// 10 = 10 ms debounce time
Bounce button2 = Bounce(2, 10);  // which is appropriate for
Bounce button3 = Bounce(3, 10);  // most mechanical pushbuttons
Bounce button4 = Bounce(4, 10);
Bounce button5 = Bounce(5, 10);  // if a button is too "sensitive"
Bounce button6 = Bounce(6, 10);  // to rapid touch, you can
Bounce button7 = Bounce(7, 10);  // increase this time.

int delaytime = 20;
void setup() {
  // Configure the pins for input mode with pullup resistors.
  // The pushbuttons connect from each pin to ground.  When
  // the button is pressed, the pin reads LOW because the button
  // shorts it to ground.  When released, the pin reads HIGH
  // because the pullup resistor connects to +5 volts inside
  // the chip.  LOW for "on", and HIGH for "off" may seem
  // backwards, but using the on-chip pullup resistors is very
  // convenient.  The scheme is called "active low", and it's
  // very commonly used in electronics... so much that the chip
  // has built-in pullup resistors!

  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);  // Teensy++ LED, may need 1k resistor pullup
  pinMode(7, INPUT_PULLUP);

}

void loop() {
  // Update all the buttons.  There should not be any long
  // delays in loop(), so this runs repetitively at a rate
  // faster than the buttons could be pressed and released.

  button2.update();
  button3.update();
  button4.update();
  button5.update();
  button6.update();
  button7.update();


  // Check each button for "falling" edge.
  // Type a message on the Keyboard when each button presses
  // Update the Joystick buttons only upon changes.
  // falling = high (not pressed - voltage from pullup resistor)
  //           to low (pressed - button connects pin to ground)

  if (button2.fallingEdge()) {
    Keyboard.press(MODIFIERKEY_CTRL);
    Keyboard.press(KEY_C);
//    Keyboard.send_now();
    delay(delaytime);
    Keyboard.releaseAll();
    }
  if (button3.fallingEdge()) {
    Keyboard.press(MODIFIERKEY_CTRL);
    Keyboard.press(KEY_V);
//    Keyboard.send_now();
    delay(delaytime);
    Keyboard.releaseAll();
    }
  if (button4.fallingEdge()) {
    Keyboard.press(MODIFIERKEY_CTRL);
    Keyboard.press(KEY_X);
//    Keyboard.send_now();
    delay(delaytime);
    Keyboard.releaseAll();
    }
  if (button5.fallingEdge()) {
    Keyboard.press(MODIFIERKEY_CTRL);
    Keyboard.press(KEY_Z);
//    Keyboard.send_now();
    delay(delaytime);
    Keyboard.releaseAll();
    }

  if (button6.fallingEdge()) {
    Keyboard.press(MODIFIERKEY_CTRL);
    Keyboard.press(KEY_Y);
//    Keyboard.send_now();
    delay(delaytime);
    Keyboard.releaseAll();
    }
  if (button7.fallingEdge()) {
    Keyboard.press(MODIFIERKEY_CTRL);
    Keyboard.press(MODIFIERKEY_SHIFT);
    Keyboard.press(KEY_V);
//    Keyboard.send_now();
    delay(delaytime);
    Keyboard.releaseAll();
    }
}
//  if (button2.risingEdge()) {
////    Keyboard.println("B2 release");
//    // release all the keys at the same instant
//    Keyboard.set_modifier(0);
//    Keyboard.set_key1(0);
//    Keyboard.send_now();
//  }
//  if (button3.risingEdge()) {
////    Keyboard.println("B3 release");
//    // release all the keys at the same instant
//    Keyboard.set_modifier(0);
//    Keyboard.set_key1(0);
//    Keyboard.send_now();
//  }
//  if (button4.risingEdge()) {
////    Keyboard.println("B4 release");
//    // release all the keys at the same instant
//    Keyboard.set_modifier(0);
//    Keyboard.set_key1(0);
//    Keyboard.send_now();
//  }
//  if (button5.risingEdge()) {
////    Keyboard.println("B5 release");
//    // release all the keys at the same instant
//    Keyboard.set_modifier(0);
//    Keyboard.set_key1(0);
//    Keyboard.send_now();
//  }
//  if (button6.risingEdge()) {
////    Keyboard.println("B6 release");
//    // release all the keys at the same instant
//    Keyboard.set_modifier(0);
//    Keyboard.set_key1(0);
//    Keyboard.send_now();
//  }
//  if (button7.risingEdge()) {
//    Keyboard.println("B7 release");
//  }
//}
