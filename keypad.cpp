#include "keypad.h"

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

void KeyPad::update() {
  unsigned int functionKey = analogRead(_functionsPin); // 0-4095
  unsigned int xKey = analogRead(_xPin); // 0-4095
  unsigned int yKey = analogRead(_yPin); // 0-4095
  unsigned int zKey = analogRead(_zPin); // 0-4095

  _keysPressed = 0; // TODO: maybe not reset

  // function keys
  if (functionKey > 3500) {
    _keysPressed += KEYCODE_A;
  }
  else if (functionKey > 2300) {
    _keysPressed += KEYCODE_B;
  }
  else if (functionKey > 1500) {
    _keysPressed += KEYCODE_C;
  }
  else if (functionKey > 500) {
    _keysPressed += KEYCODE_D;
  }

  // XY pad
  if (xKey > 3500) {
    _keysPressed += KEYCODE_RIGHT;
  }
  else if (xKey > 500) {
    _keysPressed += KEYCODE_LEFT;
  }
  if (yKey > 3500) {
    _keysPressed += KEYCODE_CENTER;
  }
  else if (yKey > 2300) {
    _keysPressed += KEYCODE_DOWN;
  }
  else if (yKey > 500) {
    _keysPressed += KEYCODE_UP;
  }

  // Z pad
  if (zKey > 3500) {
    _keysPressed += KEYCODE_ZCENTER;
  }
  else if (zKey > 2300) {
    _keysPressed += KEYCODE_ZDOWN;
  }
  else if (zKey > 500) {
    _keysPressed += KEYCODE_ZUP;
  }
}

bool KeyPad::isAnyKeyPressed() {
  return _keysPressed > 0;
}

bool KeyPad::isKeyPressed(unsigned int keyCode) {
  
}
