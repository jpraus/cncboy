#ifndef MILLING_CTRL_H
#define MILLING_CTRL_H

#include "Arduino.h"
#include "ui.h"
#include "keypad.h"

#define MILLING_RESULT_NONE 0
#define MILLING_RESULT_BACK 1

class MillingCtrl {
  public:
    MillingCtrl(UI *ui, KeyPad *keyPad);
    void start(File file);
    void stop();
    byte update(int deltaMs);

  private:
    UI &ui;
    KeyPad &keyPad;

    File file;
    int totalLines;
    int elapsedSeconds;
    byte state;

    void showReady();
    void showRunning();
    void showLoading(float progress);
    boolean isValidCommand(String line);
};

#endif
