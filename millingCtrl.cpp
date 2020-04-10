#include "millingCtrl.h"

#define STATE_READY 0
#define STATE_RUNNING 1
#define STATE_PAUSED 2

MillingCtrl::MillingCtrl(Grbl *grbl, UI *ui, KeyPad *keyPad)
    : grbl(*grbl), ui(*ui), keyPad(*keyPad) {
}

void MillingCtrl::start(File _file) {
  file = _file;
  totalLines = 0;

  String line;
  long loadedBytes = 0;
  float filesize = file.size();
  float progress = 0;
  float newProgress = 0;

  Serial.print("Reading milling ");
  Serial.println(file.name());

  showLoading(progress);

  while (file.available()) {
    line = file.readStringUntil('\n');
    if (isValidCommand(line)) {
      totalLines++;
      loadedBytes += line.length();
      newProgress = loadedBytes / filesize;
      if (newProgress - 0.02 > progress) {
        progress = newProgress;
        showLoading(progress);
      }
    }
  }

  Serial.print(totalLines);
  Serial.println(" total commands to execute");

  // done loading
  grbl.restart();
  grbl.sendReset();
  reset();
}

void MillingCtrl::stop() {
  if (file != NULL) {
    file.close();
  }
}

byte MillingCtrl::update(int deltaMs) {
  grbl.update(deltaMs);
  
  everySecondTimer -= deltaMs;
  if (everySecondTimer <= 0) {
    everySecondTimer += 1000;
    if (state == STATE_RUNNING) {
      elapsedSeconds++;
    }
  }

  if (keyPad.isKeyPressed(KEYCODE_A)) { // back
    if (state == STATE_READY) {
      return MILLING_RESULT_BACK;
    }
  }
  else if (keyPad.isKeyPressed(KEYCODE_B)) { // stop
    if (state != STATE_READY) {
      Serial.println("Stopped");
      grbl.restart();
      grbl.sendReset();
      reset(); // TODO: confirm?
    }
  }
  else if (keyPad.isKeyPressed(KEYCODE_C)) { // pause / calibrate
    if (state == STATE_RUNNING) {
      Serial.println("Paused");
      state = STATE_PAUSED;
      grbl.sendHold();
    }
    else if (state == STATE_READY) {
      return MILLING_RESULT_CALIBRATE;
    }
  }
  else if (keyPad.isKeyPressed(KEYCODE_D)) { // play
    Serial.println("Running " + String(file.name()));
    if (state == STATE_READY) {
      reset();
      grbl.sendResume();
      state = STATE_RUNNING;
    }
    else if (state == STATE_PAUSED) {
      grbl.sendResume();
      state = STATE_RUNNING;
    }
  }

  // TODO: zero out the machine function

  if (state == STATE_RUNNING && grbl.canSendCommand()) {
    sendNextCommand();
  }
  else if (grbl.isError()) {
    state = STATE_PAUSED;
    error = true;
    currentCommand = grbl.getLastCommandError();
  }

  showStatus();

  return MILLING_RESULT_NONE;
}

void MillingCtrl::reset() {
  file.seek(0); // reset file
  everySecondTimer = 0;
  elapsedSeconds = 0;
  currentLine = 0;
  currentCommand = "";
  error = false;
  state = STATE_READY;
}

void MillingCtrl::sendNextCommand() {
  String line;
  
  if (file.available()) {
    line = file.readStringUntil('\n');
    if (isValidCommand(line)) {
      line.trim();
      grbl.sendCommand(line);

      currentCommand = line;
      currentLine++;
    }
  }
  else {
    Serial.println("Done " + String(file.name()));
    currentCommand = "Finished";
    state = STATE_READY;
  }
}

void MillingCtrl::showStatus() {
  float progress = (float) currentLine / totalLines;
  MachineStatus machineStatus = grbl.getMachineStatus();

  ui.firstPage();
  do {
    ui.setFont(u8g2_font_5x8_mr);
    ui.drawStr(0, 7, file.name());
    ui.drawProgressBar(9, progress);

    ui.drawAxisIcon(0, 20);
    ui.drawStr(9, 26, "X" + String(machineStatus.x));
    ui.drawStr(49, 26, "Y" + String(machineStatus.y));
    ui.drawStr(90, 26, "Z" + String(machineStatus.z));

    ui.drawClockIcon(0, 28);
    ui.drawStr(9, 35, ui.formatTime(elapsedSeconds));
    byte len = machineStatus.state.length();
    if (len > 0) {
      ui.drawStr(128 - len * 5, 35, machineStatus.state);
    }

    ui.drawStr(0, 46, currentCommand);

    if (state == STATE_READY) {
      ui.drawTextButton(0, "Back");
      ui.drawTextButton(1, "Cal.");
    }
    else {
      ui.drawStopButton(1);
    }
    if (state == STATE_RUNNING) {
      ui.drawPauseButton(2);
    }
    else {
      ui.drawPlayButton(3);
    }
  } while (ui.nextPage());
}

void MillingCtrl::showLoading(float progress) {
  ui.firstPage();
  do {
    ui.setFont(u8g2_font_5x8_mr);
    ui.drawStr(0, 7, file.name());
    ui.drawStr(0, 20, "Reading file");
    ui.drawProgressBar(22, progress);
  } while (ui.nextPage());
}

boolean MillingCtrl::isValidCommand(String line) {
  line.trim();
  if (line.length() == 0) {
    return false;
  }
  if (line.charAt(0) == ';' || line.charAt(0) == '%' || line.charAt(0) == '(') { // skip comments
    return false;
  }
  return true;
}
