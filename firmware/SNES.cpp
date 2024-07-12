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
#include "SNES.h"

void SNESClass::begin(byte clockPin, byte strobePin, byte dataPin) {

  _clockPin = clockPin;
  _strobePin = strobePin;
  _dataPin = dataPin;

  // Setup clockPin and strobePin as OUTPUT and set both to LOW (LOW is default)
  DDR_REG |= bit(_clockPin) | bit(_strobePin);  //keep _stropePin as LOW -- LOW is default
  PORT_REG |= bit(clockPin);                    //keep _clockPin to high

  // Setup data pin as INPUT
  //bitClear(DDR_REG, _dataPin);
}

word SNESClass::update() {
  /*
       Mapping from http://www.gamesx.com/controldata/snesdat.htm
       Keys: | B Y Select Start Up Down Left Right A X  L  R N/A N/A N/A N/A |
       Bits: | 0 1   2      3    4   5    6    7   8 9 10 11  12  13  14  15 |
  */
  // clear _currentState of controller
  word _currentState_tmp;
  //HERE with reading _dataPin we can detect controller!
  bool _ctl_on = 1 ^ bitRead(PIN_REG, _dataPin);  //does not work with mouse

  /*  Serial.print(_ctl_on);
    Serial.print(" : ");
    Serial.println(connected_SNES());
  */
  _currentState_tmp = shiftInSNES();

  if (_ctl_on) {
    _currentState_tmp = ~_currentState_tmp;  //switch from logic {LOW=pressed, HIGH=released} to logic {HIGH=pressed, LOW=released}
    /*
          Default SNES controller output - output from shiftInSNES()
          //Keys: | B Y Select Start Up Down Left Right A X  L  R N/A N/A N/A N/A |
          //Bits: | 0 1   2      3    4   5    6    7   8 9 10 11  12  13  14  15 |

    */
    _currentState_tmp = _ctl_on | ((_currentState_tmp >> 1) & 511) | ((_currentState_tmp << 9) & 1536) | ((_currentState_tmp << 1) & 6144);

    /*
         processed and final OUTPUT mapping is like that:
         Keys: | CTL_ON Select Start Up Down Left Right A X B Y  L  R   N/A N/A N/A |
         Bits: |    0     1      2    3   4    5    6   7 8 9 10 11 12  13  14  15  |
    */

    _currentState_tmp &= 8191;
    /* &= B0001 1111 1111 1111; CLEAR 3 highest bits
                                There are 16 cycles but only 12 buttons + CTL_ON on first position
    */

    _currentState_tmp = nod_dpad(_currentState_tmp);
  } else {
    _currentState_tmp = 0;
  }
  return _currentState_tmp;
}

word SNESClass::nod_dpad(word _gamepadState) {
  //NO OPPOSITE DIRECTIONS
  bool OUT_DPAD_UP_tmp = nod(bitRead(_gamepadState, SNES_DPAD_UP), bitRead(_gamepadState, SNES_DPAD_DOWN));
  bool OUT_DPAD_DOWN_tmp = nod(bitRead(_gamepadState, SNES_DPAD_DOWN), bitRead(_gamepadState, SNES_DPAD_UP));
  bool OUT_DPAD_LEFT_tmp = nod(bitRead(_gamepadState, SNES_DPAD_LEFT), bitRead(_gamepadState, SNES_DPAD_RIGHT));
  bool OUT_DPAD_RIGHT_tmp = nod(bitRead(_gamepadState, SNES_DPAD_RIGHT), bitRead(_gamepadState, SNES_DPAD_DOWN));
  bitWrite(_gamepadState, SNES_DPAD_UP, OUT_DPAD_UP_tmp);
  bitWrite(_gamepadState, SNES_DPAD_DOWN, OUT_DPAD_DOWN_tmp);
  bitWrite(_gamepadState, SNES_DPAD_LEFT, OUT_DPAD_LEFT_tmp);
  bitWrite(_gamepadState, SNES_DPAD_RIGHT, OUT_DPAD_RIGHT_tmp);

  return _gamepadState;
}

/*customized version of shiftIn:
  --the strobePin shaking is moved inside,
  --strobePin and clockPin are toggled with hardware XOR
  --switched to port register manipulation,
  --adding _delay_us() -- original shiftIn is based
    on digitalRead and digitalWrite and these work also as a delay() :D
  --moving out MSBFIRST,
  --function output is "word" (not "byte") and function reads all 16 bits at once */
#pragma GCC optimize("Ofast")
word SNESClass::shiftInSNES() {
  word value = 0;
  byte _clockPin_bit = bit(_clockPin);
  byte _strobePin_bit = bit(_strobePin);
  byte _dataPin_bit = bit(_dataPin);

  noInterrupts();

  //Do the strobe to start reading button values
  PIN_REG = _strobePin_bit;  //hardware XOR
  _delay_us(2 * SNES_DELAYSTD_US);
  PIN_REG = _strobePin_bit;  //hardware XOR
  _delay_us(SNES_DELAYSTD_US);

/**************************************************/
// Here is the most time sensitive piece of code.
/**************************************************/
#pragma GCC unroll 16
  for (byte i = 0; i < 16; ++i) {  //must be 0..15 for having functional CTL_ON read
    PIN_REG = _clockPin_bit;       //hardware XOR
    _delay_us(SNES_DELAYSTD_US);
    value >>= 1;
    value |= (PIN_REG & _dataPin_bit) ? 0x8000 : 0;
    //value |= bitRead(PIN_REG, _dataPin) << i;
    PIN_REG = _clockPin_bit;  //hardware XOR
    _delay_us(SNES_DELAYSTD_US);
  }
  /**************************************************/

  interrupts();

  return value;
}

word SNESClass::getState() {
  static word _currentState;
  static unsigned long _lastReadTime;
  if ((millis() - _lastReadTime) < SNES_READ_DELAY_MS) {
    // Not enough time has elapsed, return previously read state
    return _currentState;
  }

  _currentState = update();
  _lastReadTime = millis();

  return _currentState;
}

/*function to cancel opposite directions
  eg. if UP is pressed and DOWN is pressed
  that both are considered to be NOT pressed
*/
bool SNESClass::nod(bool OUT_DIR, bool SECOND_DIR) {
  //bool value = ((!( OUT_DIR  || SECOND_DIR )) || OUT_DIR); //logic {LOW=pressed, HIGH=released}
  bool value = ((!(OUT_DIR && SECOND_DIR)) && OUT_DIR);  //logic {HIGH=pressed, LOW=released}
  return value;
}

SNESClass snes;
