#ifndef MILLING_CTRL_H
#define MILLING_CTRL_H

#include "Arduino.h"
#include "ui.h"
#include "keypad.h"

class MillingCtrl {
  public:
    MillingCtrl(UI *ui, KeyPad *keyPad);
    void start();
    void stop();
    void update(int deltaMs);

  private:
    UI &_ui;
    KeyPad &_keyPad;
};

#endif
