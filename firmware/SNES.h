#ifndef SNES_h
#define SNES_h

#define PORT_REG PORTB
#define DDR_REG DDRB
#define PIN_REG PINB

const unsigned long SNES_DELAYSTD_US = 6;
#define SNES_READ_DELAY_MS 4

enum snes_state {
  SNES_CTL_ON = 0,  // The controller is connected
  SNES_BTN_SELECT = 1,
  SNES_BTN_START = 2,
  SNES_DPAD_UP = 3,
  SNES_DPAD_DOWN = 4,
  SNES_DPAD_LEFT = 5,
  SNES_DPAD_RIGHT = 6,
  SNES_BTN_A = 7,
  SNES_BTN_X = 8,
  SNES_BTN_B = 9,
  SNES_BTN_Y = 10,
  SNES_BTN_TL = 11,
  SNES_BTN_TR = 12
};

constexpr word btr_snes_output_bits[17] = {
  bit(SNES_BTN_B),
  bit(SNES_BTN_Y),
  bit(SNES_BTN_SELECT),
  bit(SNES_BTN_START),
  bit(SNES_DPAD_UP),
  bit(SNES_DPAD_DOWN),
  bit(SNES_DPAD_LEFT),
  bit(SNES_DPAD_RIGHT),
  bit(SNES_BTN_A),
  bit(SNES_BTN_X),
  bit(SNES_BTN_TL),
  bit(SNES_BTN_TR),
  0, 0, 0, 0,
  bit(SNES_CTL_ON)
};

class SNESClass {
private:
  byte _clockPin_bit;
  byte _strobePin_bit;
  byte _dataPin_bit;
  word update();
  word shiftInSNES();
public:
  void begin(byte clockPin, byte strobePin, byte dataPin);
  word getState();
  word nod(word _input_state, word _dpad_dirs);
};

extern SNESClass snes;

#endif
