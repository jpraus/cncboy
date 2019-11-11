#include "ui.h"

#define ALIGN_LEFT 0
#define ALIGN_CENTER 2
#define ALIGN_RIGHT 5

UI::UI(U8G2 *lcd, STATE *state, byte backlightPin) 
    : _lcd(*lcd), _state(*state), _backlightPin(backlightPin) {
}

void UI::setup() {
  pinMode(_backlightPin, OUTPUT);
  digitalWrite(_backlightPin, OUTPUT);

  _lcd.setBusClock(500000);
  _lcd.begin();
  _lcd.setColorIndex(1);
  _lcd.clear();
}

void UI::update() {
  _frame = (_frame >= 1) ? 0 : _frame + 1;
  _lcd.firstPage();

  do {
    drawMillingScreen();
    //drawLogScreen();
  } while (_lcd.nextPage());
}

void UI::drawLogScreen() {
  _lcd.setFont(u8g2_font_5x8_mr);
  _lcd.setColorIndex(1);
  _lcd.drawLog(0, 7, _state.logger);
}

void UI::drawMillingScreen() {
  _lcd.setFont(u8g2_font_5x8_mr);
  _lcd.setColorIndex(1);

  float progress = 0;
  String progressStr;
  int progressBar;
  String timeStr = formatTime(_state.milling.elapsedSeconds);

  if (_state.milling.totalLines > 0) {
    progress = (float) _state.milling.currentLine / _state.milling.totalLines;
  }
  if (progress > 1) {
    progress = 1;
  }

  progressStr += (int) round(progress * 100);
  progressStr += "%";
  progressBar = 122 * progress;

  _lcd.drawStr(0, 8, _state.milling.filename.c_str());
  _lcd.drawRFrame(0, 10, 128, 10, 3);
  _lcd.drawRBox(1, 11, 4 + progressBar, 8, 2); // 5->127 (122 steps)
  _lcd.drawStr(0, 28, progressStr.c_str());
  _lcd.drawStr(128 - (5 * timeStr.length()), 28, timeStr.c_str());

  //_lcd.drawLog(0, 38, _state.logger);

  drawTextButton(0, "Load");
  drawPlayButton(1);
  drawPauseButton(2);
  drawStopButton(3);
}

void UI::drawPlayButton(byte pos) {
  int16_t x = calculateButtonLeft(pos);
  int16_t width = calculateButtonWidth(pos);

  _lcd.setColorIndex(1);
  _lcd.drawRBox(x, 52, width, 13, 2);

  _lcd.setColorIndex(0);
  _lcd.drawTriangle(x + 14, 54, x + 14, 62, x + 19, 58);
}

void UI::drawStopButton(byte pos) {
  int16_t x = calculateButtonLeft(pos);
  int16_t width = calculateButtonWidth(pos);

  drawButton(x, width);
  _lcd.setColorIndex(0);
  _lcd.drawBox(x + 13, 55, 6, 6);
}

void UI::drawPauseButton(byte pos) {
  int16_t x = calculateButtonLeft(pos);
  int16_t width = calculateButtonWidth(pos);

  drawButton(x, width);
  _lcd.setColorIndex(0);
  _lcd.drawBox(x + 14, 55, 2, 6);
  _lcd.drawBox(x + 17, 55, 2, 6);
}

void UI::drawTextButton(byte pos, String label) {
  int16_t x = calculateButtonLeft(pos);
  int16_t width = calculateButtonWidth(pos);
  byte textX = x + round((width - label.length() * 5) / 2.0);

  drawButton(x, width);
  _lcd.setColorIndex(0);
  _lcd.setFont(u8g2_font_5x8_mr);
  _lcd.drawStr(textX, 61, label.c_str());
}

void UI::drawButton(int16_t x, int16_t width) {
  _lcd.setColorIndex(1);
  _lcd.drawRBox(x, 52, width, 13, 2);
}

int16_t UI::calculateButtonLeft(byte pos) {
  return pos * 31 + pos;
}

int16_t UI::calculateButtonWidth(byte pos) {
  if (pos == 3) {
    return 32;
  }
  return 31;
}

String UI::formatTime(unsigned int seconds) {
  String time;
  String units = "min";
  byte hours = 0;
  byte minutes = 0;

  if (seconds > 3600) { // more than 1 hour
    hours = floor(seconds / 3600.0);
    seconds = seconds - hours * 3600;
  }
  if (seconds > 60) { // more than 1 minute
    minutes = floor(seconds / 60.0);
    seconds = seconds - minutes * 60;
  }

  if (hours > 0) {
    time += hours;
    time += ":";
    if (minutes < 10) {
      time += "0";
    }
    units = "h";
  }
  time += minutes;
  time += ":";
  if (seconds < 10) {
    time += "0";
  }
  time += seconds;
  time += " ";
  time += units;

  return time;
}
