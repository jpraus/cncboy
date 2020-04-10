#include "keypad.h"

#define KEYPAD_READ_TIMER 10 // millis

KeyPad::KeyPad(byte functionsPin, byte xPin, byte yPin, byte zPin)
    : functionsPin(functionsPin), xPin(xPin), yPin(yPin), zPin(zPin) {
}

void KeyPad::setup() {
  pinMode(functionsPin, INPUT);
  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(zPin, INPUT);
  
   // configure ADC
  analogReadResolution(16); // set 12bit resolution (0-4095)
  analogSetAttenuation(ADC_11db); // set AREF to be 3.3V
  analogSetCycles(32); // num of cycles per sample, 8 is default optimal
  analogSetSamples(1); // num of samples
}

void KeyPad::update(int deltaMs) {
  timer -= deltaMs;
  if (timer > 0) {
    return; // not yet
  }
  timer += KEYPAD_READ_TIMER;

  unsigned int functionKey = analogRead(functionsPin); // 0-4095
  unsigned int xKey = analogRead(xPin); // 0-4095
  unsigned int yKey = analogRead(yPin); // 0-4095
  unsigned int zKey = analogRead(zPin); // 0-4095

  keysDown = 0; // TODO: maybe not reset

  // function keys
  if (functionKey > 3500) {
    keysDown += KEYCODE_D;
  }
  else if (functionKey > 2300) {
    keysDown += KEYCODE_C;
  }
  else if (functionKey > 1800) {
    keysDown += KEYCODE_B;
  }
  else if (functionKey > 500) {
    keysDown += KEYCODE_A;
  }

  // XY pad
  if (xKey > 3500) {
    keysDown += KEYCODE_RIGHT;
  }
  else if (xKey > 500) {
    keysDown += KEYCODE_LEFT;
  }
  if (yKey > 3500) {
    keysDown += KEYCODE_CENTER;
  }
  else if (yKey > 2300) {
    keysDown += KEYCODE_DOWN;
  }
  else if (yKey > 500) {
    keysDown += KEYCODE_UP;
  }

  // Z pad
  if (zKey > 3500) {
    keysDown += KEYCODE_ZCENTER;
  }
  else if (zKey > 2300) {
    keysDown += KEYCODE_ZDOWN;
  }
  else if (zKey > 500) {
    keysDown += KEYCODE_ZUP;
  }

  // unflag keys up
  keysRead = keysRead & keysDown;
}

bool KeyPad::isAnyKeyPressed() {
  return keysDown > 0;
}

bool KeyPad::isKeyPressed(unsigned int keyCode) {
  if ((keysDown & keyCode) > 0 && (keysRead & keyCode) == 0) {
    keysDown -= keyCode;
    keysRead += keyCode; // ack key pressed -> do not show as pressed until key up and down again
    return true;
  }
  return false;
}
