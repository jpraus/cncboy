#include "keypad.h"

#define KEYPAD_READ_TIMER 10 // millis

KeyPad::KeyPad(byte functionsPin, byte xPin, byte yPin, byte zPin)
    : _functionsPin(functionsPin), _xPin(xPin), _yPin(yPin), _zPin(zPin) {
}

void KeyPad::setup() {
  pinMode(_functionsPin, INPUT);
  pinMode(_xPin, INPUT);
  pinMode(_yPin, INPUT);
  pinMode(_zPin, INPUT);
  
   // configure ADC
  analogReadResolution(16); // set 12bit resolution (0-4095)
  analogSetAttenuation(ADC_11db); // set AREF to be 3.3V
  analogSetCycles(32); // num of cycles per sample, 8 is default optimal
  analogSetSamples(1); // num of samples
}

void KeyPad::update(int deltaMs) {
  _timer -= deltaMs;
  if (_timer > 0) {
    return; // not yet
  }
  _timer += KEYPAD_READ_TIMER;

  unsigned int functionKey = analogRead(_functionsPin); // 0-4095
  unsigned int xKey = analogRead(_xPin); // 0-4095
  unsigned int yKey = analogRead(_yPin); // 0-4095
  unsigned int zKey = analogRead(_zPin); // 0-4095

  _keysDown = 0; // TODO: maybe not reset

  // function keys
  if (functionKey > 3500) {
    _keysDown += KEYCODE_A;
  }
  else if (functionKey > 2300) {
    _keysDown += KEYCODE_B;
  }
  else if (functionKey > 1500) {
    _keysDown += KEYCODE_C;
  }
  else if (functionKey > 500) {
    _keysDown += KEYCODE_D;
  }

  // XY pad
  if (xKey > 3500) {
    _keysDown += KEYCODE_RIGHT;
  }
  else if (xKey > 500) {
    _keysDown += KEYCODE_LEFT;
  }
  if (yKey > 3500) {
    _keysDown += KEYCODE_CENTER;
  }
  else if (yKey > 2300) {
    _keysDown += KEYCODE_DOWN;
  }
  else if (yKey > 500) {
    _keysDown += KEYCODE_UP;
  }

  // Z pad
  if (zKey > 3500) {
    _keysDown += KEYCODE_ZCENTER;
  }
  else if (zKey > 2300) {
    _keysDown += KEYCODE_ZDOWN;
  }
  else if (zKey > 500) {
    _keysDown += KEYCODE_ZUP;
  }

  // unflag keys up
  _keysRead = _keysRead & _keysDown;
}

bool KeyPad::isAnyKeyPressed() {
  return _keysDown > 0;
}

bool KeyPad::isKeyPressed(unsigned int keyCode) {
  if ((_keysDown & keyCode) > 0 && (_keysRead & keyCode) == 0) {
    _keysDown -= keyCode;
    _keysRead += keyCode; // ack key pressed -> do not show as pressed until key up and down again
    return true;
  }
  return false;
}
