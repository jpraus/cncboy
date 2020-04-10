#ifndef MILLING_CTRL_H
#define MILLING_CTRL_H

#include "Arduino.h"
#include "ui.h"
#include "keypad.h"

#define MILLING_RESULT_NONE 0
#define MILLING_RESULT_BACK 1

struct MachineStatus {
  String state;
  float x;
  float y;
  float z;
};

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
    byte state;

    int everySecondTimer;
    int elapsedSeconds;
    int currentLine;
    String currentCommand;
    bool commandAwaitReply;
    bool error;
    bool queryStatus;
    MachineStatus machineStatus;

    void reset();
    void sendNextCommand();
    void sendCommand(String command);
    void receiveResponse();
    void parseStatusReport(String report);
    void showStatus();
    void showLoading(float progress);
    boolean isValidCommand(String line);
};

#endif
