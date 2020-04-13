#ifndef KEYPAD_H
#define KEYPAD_H

#include "Arduino.h"

#define KEYCODE_A 0
#define KEYCODE_B 1
#define KEYCODE_C 2
#define KEYCODE_D 3

#define KEYCODE_UP 4
#define KEYCODE_LEFT 5
#define KEYCODE_RIGHT 6
#define KEYCODE_DOWN 7
#define KEYCODE_CENTER 8

#define KEYCODE_ZUP 9
#define KEYCODE_ZDOWN 10
#define KEYCODE_ZCENTER 11

class KeyPad {
  public:
    KeyPad(byte functionsPin, byte xPin, byte yPin, byte zPin);
    void setup();
    void update();
    bool isAnyKeyPressed();
    bool isKeyPressed(byte keyCode);
    unsigned int isKeyHold(byte keyCode);

  private:
    byte functionsPin;
    byte xPin;
    byte yPin;
    byte zPin;
    unsigned long millisRef = 0;

    unsigned int keysDown = 0;
    unsigned int keysRead = 0;

    unsigned int keyCodeMap[12] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048};
    unsigned int keyHoldTime[12];
};

#endif
