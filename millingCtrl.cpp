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
  grbl.sendCommand("G91"); // relative coordinates for calibration
  reset();
}

void MillingCtrl::stop() {
  if (file != NULL) {
    file.close();
  }
}

byte MillingCtrl::update() {
  grbl.update();

  if (millis() >= millisRef + 1000) { // everysecond timer
    millisRef = millis();
    redraw = true;
    if (state == STATE_RUNNING) {
      elapsedSeconds++;
    }
  }

  if (keyPad.isKeyPressed(KEYCODE_A)) { // back
    if (state == STATE_READY) {
      return MILLING_RESULT_BACK;
    }
  }
  else if (keyPad.isKeyPressed(KEYCODE_B)) { // stop / home
    if (state != STATE_READY) {
      Serial.println("Stopped");
      grbl.restart();
      grbl.sendReset();
      reset(); // TODO: confirm?
    }
    else { // homing
      // TODO: command sequence handling (await ACK)
      grbl.sendCommand("G90");
      grbl.sendCommand("G0 X0 Y0");
      grbl.sendCommand("G91");
      // todo prevent multiple calls
    }
  }
  else if (keyPad.isKeyPressed(KEYCODE_C)) { // pause
    if (state == STATE_RUNNING) {
      Serial.println("Paused");
      state = STATE_PAUSED;
      grbl.sendHold();
      redraw = true;
    }
  }
  else if (keyPad.isKeyPressed(KEYCODE_D)) { // play
    Serial.println("Running " + String(file.name()));
    if (state == STATE_READY) {
      reset();
      grbl.sendResume();
      state = STATE_RUNNING;
      redraw = true;
    }
    else if (state == STATE_PAUSED) {
      grbl.sendResume();
      state = STATE_RUNNING;
      redraw = true;
    }
  }
  else if (state == STATE_READY) {
    calibration();
  }
  

  // TODO: zero out the machine function

  if (state == STATE_RUNNING && grbl.canSendCommand()) {
    sendNextCommand();
  }
  else if (grbl.isError()) {
    state = STATE_PAUSED;
    error = true;
    currentCommand = grbl.getLastCommandError();
    redraw = true;
  }

  if (redraw) {
    showStatus();
    redraw = false;
    //Serial.println("DEBUG: Redraw");  
  }

  return MILLING_RESULT_NONE;
}

void MillingCtrl::calibration() {
  float x = 0;
  float nx = 0;
  float y = 0;
  float ny = 0;
  float z = 0;
  float nz = 0;
  
  if (keyPad.isKeyPressed(KEYCODE_CENTER)) {
    // set XY-home
    // TODO: command sequence handling (await ACK)
    grbl.sendCommand("G90");
    grbl.sendCommand("G10 L20 P1 X0 Y0");
    grbl.sendCommand("G91");
    return;
  }
  if (keyPad.isKeyPressed(KEYCODE_ZCENTER)) {
    // set Z-home
    // TODO: command sequence handling (await ACK)
    grbl.sendCommand("G90");
    grbl.sendCommand("G10 L20 P1 Z0");
    grbl.sendCommand("G91");
    return;
  }

  y = calibrationStep(keyPad.isKeyHold(KEYCODE_UP), 0, 800);
  ny = calibrationStep(keyPad.isKeyHold(KEYCODE_DOWN), 1, 800);
  x = calibrationStep(keyPad.isKeyHold(KEYCODE_RIGHT), 2, 800);
  nx = calibrationStep(keyPad.isKeyHold(KEYCODE_LEFT), 3, 800);
  z = calibrationStep(keyPad.isKeyHold(KEYCODE_ZUP), 4, 200);
  nz = calibrationStep(keyPad.isKeyHold(KEYCODE_ZDOWN), 5, 200);

  String command;
  if (x > 0) {
    command += " X" + String(x);
  }
  else if (x < 0 && nx > 0) {
    command += " X-" + String(nx);
  }
  if (y > 0) {
    command += " Y" + String(y);
  }
  else if (y < 0 && ny > 0) {
    command += " Y-" + String(ny);
  }
  if (z > 0) {
    command += " Z" + String(z);
  }
  else if (z < 0 && nz > 0) {
    command += " Z-" + String(nz);
  }
  if (command.length() > 0) {
    grbl.sendCommand("G0" + command);
    redraw = true;
  }
}

float MillingCtrl::calibrationStep(unsigned int holdDownTime, byte index, unsigned int mmPerMinute) {
  unsigned int treshold = calibrationAccelTresholds[index];
  float step;
  float mmPerHalfSecond = mmPerMinute / 120.0;

  // 800mm/min (14mm/s)
  if (holdDownTime == 0) {
    calibrationAccelTresholds[index] = 0; // reset
    return -1; // no move in this direction
  }

  if (holdDownTime < treshold) {
    return 0; // treshold not reached, but moving in this direction
  }

  if (treshold == 0) {
    treshold = 200;
    step = 0.1;
  }
  else if (treshold == 200) {
    treshold = 400;
    step = 0.5;
  }
  else if (treshold == 400) {
    treshold = 600;
    step = ceil(mmPerHalfSecond * 10 / 4) / 10.0;
  }
  else if (treshold == 600) {
    treshold = 800;
    step = ceil(mmPerHalfSecond * 10 / 2) / 10.0;
  }
  else if (treshold >= 800) {
    treshold += 500;
    step = ceil(mmPerHalfSecond * 10) / 10.0;
  }

  calibrationAccelTresholds[index] = treshold;
  return step;
}

void MillingCtrl::reset() {
  file.seek(0); // reset file
  elapsedSeconds = 0;
  currentLine = 0;
  currentCommand = "";
  error = false;
  state = STATE_READY;
  redraw = true;
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
      redraw = true;
    }
  }
  else {
    Serial.println("Done " + String(file.name()));
    currentCommand = "Finished";
    state = STATE_READY;
    redraw = true;
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
      ui.drawTextButton(1, "Home");
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
