//#pragma GCC optimize ("Ofast,unroll-loops,inline")
#ifndef SNES_h
#define SNES_h

#define PORT_REG PORTB
#define DDR_REG DDRB
#define PIN_REG PINB

const unsigned long SNES_DELAYSTD_US = 6;
const unsigned long SNES_READ_DELAY_MS = 4;

enum snes_state {
  SNES_CTL_ON =      0,  // The controller is connected
  SNES_BTN_SELECT =  1,
  SNES_BTN_START =   2,
  SNES_DPAD_UP =     3,
  SNES_DPAD_DOWN =   4,
  SNES_DPAD_LEFT =   5,
  SNES_DPAD_RIGHT =  6,
  SNES_BTN_A =       7,
  SNES_BTN_X =       8,
  SNES_BTN_B =       9,
  SNES_BTN_Y =      10,
  SNES_BTN_TL =     11,
  SNES_BTN_TR =     12
};

class SNESClass {
private:
  byte _clockPin;
  byte _strobePin;
  byte _dataPin;
  word update();
  word shiftInSNES();
  word nod_dpad(word _gamepadState);
public:
  void begin(byte clockPin, byte strobePin, byte dataPin);
  word getState();
  bool nod(bool OUT_DIR, bool SECOND_DIR);
};

extern SNESClass snes;

#endif
