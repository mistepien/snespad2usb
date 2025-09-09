/*********************************************
   Created by Michał Stępień
   Contact: mistepien@wp.pl
   License: GPLv3
 *********************************************/

/*----------------------------------------
  |   DDRx   |   PORTx  |    result      |
  ---------------------------------------|
  ---------------------------------------|
  |    0     |     0    |     INPUT      |
  ----------------------------------------
  |    0     |     1    |  INPUT_PULLUP  |
  ----------------------------------------
  |    1     |     0    | OUTPUT (LOW)   |
  ----------------------------------------
  |    1     |     1    | OUTPUT (HIGH)  |
  ----------------------------------------

  LOW(0) state of both registers is DEFAULT,
  thus every pin is in INPUT mode without doing anything.

  ------------------
  |  HARDWARE XOR  |
  ------------------
  PINx = byte;      <======>    PORTx ^= byte;

  "The port input pins I/O location is read only, while the data register and the
  data direction register are read/write. However, writing a logic one to a bit
  in the PINx register, will result in a toggle in the
  corresponding bit in the data register."

  That is more efficient since XOR operation is done in hardware, not software,
  and it saves cycles since in code there is no need to bother about XOR.*/

#include "Arduino.h"
#if defined(__AVR_ATtiny88__) || defined(__AVR_ATtiny48__)
#include <util/delay.h>
#endif
#include "SNES.h"

void SNESClass::begin(byte clockPin, byte strobePin, byte dataPin) {
  // Setup clockPin and strobePin as OUTPUT
  DDR_REG |= bit(clockPin) | bit(strobePin);  //keep _stropePin as LOW -- LOW is default
  PORT_REG |= bit(clockPin);                  //keep _clockPin to high

  // Setup data pin as INPUT -- that is DEFAULT
  //bitClear(DDR_REG, dataPin);

  _clockPin_bit = bit(clockPin);
  _strobePin_bit = bit(strobePin);
  _dataPin_bit = bit(dataPin);
}

word SNESClass::update() {
  /*
       Mapping from http://www.gamesx.com/controldata/snesdat.htm
       Keys: | B Y Select Start Up Down Left Right A X  L  R N/A N/A N/A N/A |
       Bits: | 0 1   2      3    4   5    6    7   8 9 10 11  12  13  14  15 |
  */

  /*
       to final OUTPUT mapping is like that:
       Keys: | CTL_ON Select Start Up Down Left Right A X B Y  L  R   N/A N/A N/A |
       Bits: |    0     1      2    3   4    5    6   7 8 9 10 11 12  13  14  15  |
  */

  word _currentState_tmp = shiftInSNES();

  if (_currentState_tmp & bit(SNES_CTL_ON)) {
    _currentState_tmp = nod(_currentState_tmp, bit(SNES_DPAD_UP) | bit(SNES_DPAD_DOWN));     //NO OPPOSITE DIRECTIONS
    _currentState_tmp = nod(_currentState_tmp, bit(SNES_DPAD_RIGHT) | bit(SNES_DPAD_LEFT));  //NO OPPOSITE DIRECTIONS
  } else {
    _currentState_tmp = 0;
  }

  return _currentState_tmp;
}

/*customized version of shiftIn:
  --the strobePin shaking is moved inside,
  --strobePin and clockPin are toggled with hardware XOR
  --switched to port register manipulation,
  --adding _delay_us() -- original shiftIn is based
    on digitalRead and digitalWrite and these work also as a delay() :D
  --moving out MSBFIRST,
  --switch from logic {LOW=pressed, HIGH=released} to logic {HIGH=pressed, LOW=released},
  --function output is "word" (not "byte") and function reads all 16 bits at once
  --functions returns is gamepad connected or not,
  --reorder output using table btr_snes_output_bits[17] */
word SNESClass::shiftInSNES() {
  word value = 0;

  noInterrupts();

  /**************************************************/
  // Here is the most time sensitive piece of code.
  /**************************************************/

  //Do the strobe to start reading button values
  PIN_REG = _strobePin_bit;  //hardware XOR
  _delay_us(2 * SNES_DELAYSTD_US);
  PIN_REG = _strobePin_bit;  //hardware XOR
  _delay_us(SNES_DELAYSTD_US);

  //#pragma GCC unroll 17
  for (byte i = 0; i < 17; i++) { /* must be 0..16 for having functional CTL_ON read
                                    last cycle is about CTL_ON
                                    */
    PIN_REG = _clockPin_bit;      //hardware XOR
#if (F_CPU > 1000000L)
    _delay_us(SNES_DELAYSTD_US);
#endif
    value |= PIN_REG & _dataPin_bit ? 0 : btr_snes_output_bits[i];
    PIN_REG = _clockPin_bit;  //hardware XOR
    _delay_us(SNES_DELAYSTD_US);
  }

  /**************************************************/

  interrupts();

  return value;
}

word SNESClass::getState() {
#if SNES_READ_DELAY_MS > 0
  static word _currentState;
  static unsigned long _lastReadTime;
  if ((millis() - _lastReadTime) < SNES_READ_DELAY_MS) {
    // Not enough time has elapsed, return previously read state
    return _currentState;
  }
#else
  word _currentState;
#endif

  _currentState = update();

#if SNES_READ_DELAY_MS > 0
  _lastReadTime = millis();
#endif

  return _currentState;
}

/*function to cancel opposite directions
  eg. if UP is pressed and DOWN is pressed
  that both are considered to be NOT pressed
*/
word SNESClass::nod(word _input_state, word _dpad_dirs) {
  word xor_output = (_input_state & _dpad_dirs) ^ _dpad_dirs;
  return xor_output ? _input_state : (_input_state & ~(_dpad_dirs));
}

SNESClass snes;