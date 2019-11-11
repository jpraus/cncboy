#ifndef KEYPAD_H
#define KEYPAD_H

#include "Arduino.h"

#define KEYCODE_NONE 0

#define KEYCODE_A 1
#define KEYCODE_B 2
#define KEYCODE_C 4
#define KEYCODE_D 8

#define KEYCODE_UP 16
#define KEYCODE_LEFT 32
#define KEYCODE_RIGHT 64
#define KEYCODE_DOWN 128
#define KEYCODE_CENTER 256

#define KEYCODE_ZUP 512
#define KEYCODE_ZDOWN 1024
#define KEYCODE_ZCENTER 2048

class KeyPad {
  public:
    KeyPad(byte functionsPin, byte xPin, byte yPin, byte zPin);
    void setup();
    void update();
    bool isAnyKeyPressed();
    bool isKeyPressed(unsigned int keyCode);

  private:
    byte _functionsPin;
    byte _xPin;
    byte _yPin;
    byte _zPin;
    unsigned int _keysPressed;

};

#endif
