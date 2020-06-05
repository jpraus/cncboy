#include "millingCtrl.h"

void Grbl::start() {
  initLogFile();
  bufferSize = GRBL_BUFFER_SIZE;
}

void Grbl::restart() {
  initLogFile();
  bufferSize = GRBL_BUFFER_SIZE;
}

byte Grbl::update(unsigned long nowMillis) {
  if (nowMillis >= millisRef + 1000) { // everysecond timer
    millisRef = nowMillis;
    enqueueCommand("?", true);
    logFile.flush();
  }

  receiveResponse();

  // try to send the last command that did not fit into the buffer
  if (nextCommand != "") {
    if (enqueueCommand(nextCommand)) {
      nextCommand = "";
    }
  }
}

bool Grbl::isError() {
  return error;
}

void Grbl::clearError() {
  error = false;
}

String Grbl::getLastCommandError() {
  return errorMessage;
}

MachineStatus Grbl::getMachineStatus() {
  return machineStatus;
}

void Grbl::sendHold() {
  sendCommand("!"); // no reply on hold command
}

void Grbl::sendResume() {
  enqueueCommand("~", true);
}

void Grbl::sendReset() {
  enqueueCommand(String(char(18)));
}

bool Grbl::canEnqueueCommand() {
  return nextCommand == "" && !error;
}

bool Grbl::enqueueCommand(String command, bool realTime) {
  byte len = command.length() + 1; // end character is not part of the command but will occupy the buffer
  byte allowLen = realTime ? len : len + 1; // always keep a room for real-time command

  if (bufferSize > allowLen) {
    bufferSize -= len;
    streamedCommandsLens[streamedCommandsHead++] = len;
    if (streamedCommandsHead == GRBL_BUFFER_SIZE) {
      streamedCommandsHead = 0;
    }

    Serial.print("Buffer: ");
    Serial.println(bufferSize);

    sendCommand(command);
    return true;
  }
  else if (!realTime) {
    nextCommand = command;
    return false; // do not enqueue any new command
  }
}

void Grbl::sendCommand(String command) {
  Serial2.print(command);
  Serial2.print('\n');
  //Serial2.flush();

  Serial.println(command);

  logFile.print(millis());
  logFile.print(" > ");
  logFile.println(command);

  //lastCommand = command;
  //awaitingReply = true;
  //error = false;
  //errorMessage = "";
  //partialResponse = "";
  //retrying = false;
}

void Grbl::receiveResponse() {
  String response;
  bool popBuffer = false;

  if (Serial2.available()) {
    response = Serial2.readStringUntil('\n');
  }

  if (response.length() > 0) {
    logFile.print(millis());
    logFile.print(" < ");
    logFile.println(response);

    if (response.startsWith("ok")) {
      // ok response
      Serial.println("OK");
      popBuffer = true;
    }
    else if (response.startsWith("error")) {
      // error response
      Serial.println("Error: " + response);
      errorMessage = response;
      error = true;
      popBuffer = true;
    }
    else if (response.startsWith("<")) {
      // machine status report
      Serial.println("Report: " + response);
      parseStatusReport(response);
    }
    else {
      Serial.println("Unexpected: " + response);
    }

    // response received, pop one item from buffer
    if (popBuffer) {
      bufferSize += streamedCommandsLens[streamedCommandsTail++];
      if (streamedCommandsTail == GRBL_BUFFER_SIZE) {
        streamedCommandsTail = 0;
      }
      if (bufferSize > GRBL_BUFFER_SIZE) {
        bufferSize = GRBL_BUFFER_SIZE;
      }
  
      Serial.print("Buffer: ");
      Serial.println(bufferSize);
    }
  }
}

void Grbl::parseStatusReport(String report) {
  char token;
  String value;

  byte valueIndex = 0;
  byte coordinateIndex = 0;

  // format: <Run|MPos:-0.950,-4.887,-2.500|FS:1010,1000>

  for (int i = 0; i < report.length(); i++) {
    token = report.charAt(i);
    if (token == '<') {
      continue; // start
      value = "";
    }
    else if (token == '|' || token == ':') {
      if (valueIndex == 0) { // machine status
        machineStatus.state = value;
        value = "";
        valueIndex++;
      }
      else if (valueIndex == 3) { // z coordinate
        machineStatus.z = value.toFloat();
        value = "";
        valueIndex++;
      }
      else {
        value = "";
      }
    }
    else if (token == ',') {
      if (valueIndex == 1) { // x coordinate
        machineStatus.x = value.toFloat();
        value = "";
        valueIndex++;
      }
      else if (valueIndex == 2) { // y coordinate
        machineStatus.y = value.toFloat();
        value = "";
        valueIndex++;
      }
      else {
        value = "";
      }
    }
    else if (token == '>') {
      break; // stop
    }
    else {
      value += token;
    }
  }
}

void Grbl::initLogFile() {
  String logFilename;
  int counter = 1;

  // find new unique name for the log file
  do {
    logFilename = "/logs/" + String(counter) + ".log";
    counter++;
  } while (SD.exists(logFilename));

  logFile.close();
  logFile = SD.open(logFilename, FILE_WRITE);

  Serial.print("Logging to file ");
  Serial.println(logFile.name());
}
