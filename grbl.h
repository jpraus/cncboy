#ifndef GRBL_H
#define GRBL_H

#include "Arduino.h"

struct MachineStatus {
  String state;
  float x;
  float y;
  float z;
};

class Grbl {
  public:
    byte update(int deltaMs);
    void restart();

    bool canSendCommand();
    bool isError();
    String getLastCommandError();
    void sendCommand(String command);
    void sendHold();
    void sendResume();
    void sendReset();
    MachineStatus getMachineStatus();

  private:
    int everySecondTimer;
    bool awaitingReply;
    bool error;
    String errorMessage;
    bool queryStatus;
    MachineStatus machineStatus;

    void receiveResponse();
    void parseStatusReport(String report);
};

#endif
