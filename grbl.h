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
    void start();
    byte update(unsigned long nowMillis);
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
    unsigned long millisRef;
    String lastCommand;
    bool awaitingReply;
    String partialResponse;
    bool error;
    bool retrying;
    String errorMessage;
    bool queryStatus;
    MachineStatus machineStatus;
    File logFile;

    void receiveResponse();
    void parseStatusReport(String report);
};

#endif
