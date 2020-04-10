#ifndef STATE_H
#define STATE_H

#include "Arduino.h"
#include <SD.h>

#define U8LOG_WIDTH 25
#define U8LOG_HEIGHT 8

struct FileDescriptor {
  String filename;
  unsigned int totalLines;
  File file;
  unsigned int currentLine;
};

struct MillingStatus {
  String filename;
  unsigned long totalLines;
  unsigned long currentLine;
  unsigned int elapsedSeconds;
};

class STATE {
  public:
    STATE() {
      logger.begin(U8LOG_WIDTH, U8LOG_HEIGHT, loggerBuffer);
      logger.setLineHeightOffset(1);
    }

    U8G2LOG logger;
    MillingStatus milling; // read-write
    FileDescriptor openFile; // read-write

  private:
    uint8_t loggerBuffer[U8LOG_WIDTH * U8LOG_HEIGHT];
};

#endif
