#include "SNES.h"


#include <Joystick.h>  //https://github.com/MHeironimus/ArduinoJoystickLibrary
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK,
                   9, 0,                  // Button Count, Hat Switch Count
                   true, true, false,     // X and Y, but no Z Axis
                   false, false, false,   // No Rx, Ry, or Rz
                   false, false,          // No rudder or throttle
                   false, false, false);  // No accelerator, brake, or steering

void setup() {
  bitSet(DDRE, 6); bitClear(PORTE, 6); //STATUS LED - OFF
  snes.begin(1, 2, 3);

  Joystick.setXAxisRange(-1, 1);
  Joystick.setYAxisRange(-1, 1);
  Joystick.begin(false);
  Joystick.setXAxis(0);
  Joystick.setYAxis(0);

  delay(1500); /*very ugly and dirty hack
                without that delay() joystick will not
                be centered at the beginning (that is an
                issue with Joystick.sendState();
*/


  /*turn off RX and TX LEDS on permanent basis

    turning off these LEDS here at the end of setup()
    is an indicator that joy2usb is ready
  */

  bitClear(DDRD, 5);
  bitClear(DDRB, 0);

  bitSet(DDRC, 6); bitClear(PORTC, 6); //CTL-ON LED - OFF
  bitSet(PORTE, 6); //STATUS LED - ON

  bitClear(DDRD, 0);
  bitSet(PORTD, 0);

  Joystick.sendState();
}

byte DPAD_UP;
byte DPAD_DOWN;
byte DPAD_LEFT;
byte DPAD_RIGHT;

void button(byte btn, bool btn_state) {
  switch (btn) {
    case SNES_CTL_ON:
      bitWrite(PORTC, 6, btn_state); //CTL-ON LED
      break;
    case SNES_BTN_SELECT:
      Joystick.setButton(0, btn_state);
      break;
    case SNES_BTN_START:
      Joystick.setButton(1, btn_state);
      break;
    case SNES_DPAD_UP:
      DPAD_UP = btn_state;
      break;
    case SNES_DPAD_DOWN:
      DPAD_DOWN = btn_state;
      break;
    case SNES_DPAD_LEFT:
      DPAD_LEFT = btn_state;
      break;
    case SNES_DPAD_RIGHT:
      DPAD_RIGHT = btn_state;
      break;
    case SNES_BTN_A:
      Joystick.setButton(2, btn_state);
      break;
    case SNES_BTN_X:
      Joystick.setButton(3, btn_state);
      break;
    case SNES_BTN_B:
      Joystick.setButton(4, btn_state);
      break;
    case SNES_BTN_Y:
      Joystick.setButton(5, btn_state);
      break;
    case SNES_BTN_TL:
      Joystick.setButton(6, btn_state);
      break;
    case SNES_BTN_TR:
      Joystick.setButton(7, btn_state);
      break;
  }
}

word current_state = 0;
word prev_state = 0;


constexpr int hotkey_debounce_time_MS = 100;
void loop() {
  while (true) {
    current_state = snes.getState();
    word changed_state = prev_state ^ current_state;
    if (changed_state) {
      prev_state = current_state;
      for (byte index = 0; index < 13; index++) {
        if (bitRead(changed_state, index)) {
          button(index, bitRead(current_state, index));
        }
        Joystick.setYAxis(DPAD_DOWN - DPAD_UP);
        Joystick.setXAxis(DPAD_RIGHT - DPAD_LEFT);
      }
    }

    static unsigned long prev_hotkey_time;
    static bool prev_hotkey_state;
    bool current_hotkey_state = 1 ^ bitRead(PIND, 0);
    if ( current_hotkey_state ^ prev_hotkey_state ) {
      unsigned long current_hotkey_time = millis();
      if (current_hotkey_time - prev_hotkey_time > hotkey_debounce_time_MS) {
        prev_hotkey_state = current_hotkey_state;
        prev_hotkey_time = current_hotkey_time;
        Joystick.setButton(8, current_hotkey_state);
      }
    }
    Joystick.sendState();  //ONE common send.State for AXISES AND BUTTONS
  }
}
