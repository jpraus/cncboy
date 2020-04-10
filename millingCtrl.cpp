#include "millingCtrl.h"

#define STATE_READY 0
#define STATE_RUNNING 1
#define STATE_PAUSED 2

MillingCtrl::MillingCtrl(UI *ui, KeyPad *keyPad) 
    : ui(*ui), keyPad(*keyPad) {
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
  file.seek(0); // reset file
  elapsedSeconds = 0;
  state = STATE_READY;
}

void MillingCtrl::stop() {
  if (file != NULL) {
    file.close();
  }
}

byte MillingCtrl::update(int deltaMs) {
  if (keyPad.isKeyPressed(KEYCODE_A)) { // back
    if (state == STATE_READY) {
      return MILLING_RESULT_BACK;
    }
  }
  else if (keyPad.isKeyPressed(KEYCODE_B)) { // stop
    state = STATE_READY; // confirm?
    // reset
  }
  else if (keyPad.isKeyPressed(KEYCODE_C)) { // pause
    if (state == STATE_RUNNING) {
      state = STATE_PAUSED;
    }
  }
  else if (keyPad.isKeyPressed(KEYCODE_D)) { // play
    Serial.println("pressed");
    if (state == STATE_READY) {
      state = STATE_RUNNING;
    }
  }

  // TODO: zero out the machine function

  switch (state) {
    case STATE_READY:
      showReady();
      break;
    case STATE_RUNNING:
      showRunning();
      break;
    case STATE_PAUSED:
      break;
  }

  return MILLING_RESULT_NONE;
}

void MillingCtrl::showReady() {
  ui.firstPage();
  do {
    ui.setFont(u8g2_font_5x8_mr);
    ui.drawStr(0, 7, file.name());
    ui.drawStr(0, 16, "Ready " + String(totalLines) + " commands");
    //ui.drawProgressBar(9, progress);

    // commands history
    // elapsed time ? estimated time?

    ui.drawTextButton(0, "Back");
    ui.drawPlayButton(3);
  } while (ui.nextPage());
}

void MillingCtrl::showRunning() {
  ui.firstPage();
  do {
    ui.setFont(u8g2_font_5x8_mr);
    ui.drawStr(0, 7, file.name());
    ui.drawStr(0, 16, String(totalLines) + " commands");
    //ui.drawProgressBar(9, progress);

    // commands history
    // elapsed time ? estimated time?
    
    ui.drawStopButton(1);
    ui.drawPauseButton(2);
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
