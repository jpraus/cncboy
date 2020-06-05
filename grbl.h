#ifndef GRBL_H
#define GRBL_H

#include "Arduino.h"

#define GRBL_BUFFER_SIZE 127

struct MachineStatus {
  String state;
  float x;
  float y;
  float z;
};

class Grbl {
  public:
    void start();
    void restart();
    byte update(unsigned long nowMillis);

    bool isError();
    String getLastCommandError();
    void clearError();
    bool canEnqueueCommand();
    bool enqueueCommand(String command, bool realTime=false);
    void sendHold();
    void sendResume();
    void sendReset();
    MachineStatus getMachineStatus();

  private:
    unsigned long millisRef;
    String lastCommand;
    String nextCommand;
    bool error;
    bool retrying;
    String errorMessage;
    bool queryStatus;

    // streaming protocol counters
    byte bufferSize = GRBL_BUFFER_SIZE;
    byte streamedCommandsTail = 0;
    byte streamedCommandsHead = 0;
    byte streamedCommandsLens[GRBL_BUFFER_SIZE];

    MachineStatus machineStatus;
    File logFile;

    void sendCommand(String command);
    void receiveResponse();
    void parseStatusReport(String report);
    void initLogFile();
};

#endif
