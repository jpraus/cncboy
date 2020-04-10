#ifndef MILLING_CTRL_H
#define MILLING_CTRL_H

#include "Arduino.h"
#include <SD.h>
#include "ui.h"
#include "keypad.h"
#include "grbl.h"

#define MILLING_RESULT_NONE 0
#define MILLING_RESULT_BACK 1
#define MILLING_RESULT_CALIBRATE 2

class MillingCtrl {
  public:
    MillingCtrl(Grbl *grbl, UI *ui, KeyPad *keyPad);
    void start(File file);
    void stop();
    byte update(int deltaMs);

  private:
	Grbl &grbl;
    UI &ui;
    KeyPad &keyPad;

    File file;
    int totalLines;
    byte state;

    int everySecondTimer;
    int elapsedSeconds;
    int currentLine;
    String currentCommand;
    bool error;

    void reset();
    void sendNextCommand();
    void showStatus();
    void showLoading(float progress);
    boolean isValidCommand(String line);
};

#endif
