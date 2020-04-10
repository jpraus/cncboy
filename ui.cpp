#include "ui.h"

#define ALIGN_LEFT 0
#define ALIGN_CENTER 2
#define ALIGN_RIGHT 5

UI::UI(U8G2 *lcd, STATE *state, byte backlightPin) 
    : lcd(*lcd), state(*state), backlightPin(backlightPin) {
}

void UI::setup() {
  pinMode(backlightPin, OUTPUT);
  digitalWrite(backlightPin, OUTPUT);

  lcd.setBusClock(500000);
  lcd.begin();
  lcd.setColorIndex(1);
  lcd.clear();
}

void UI::update() {
  frame = (frame >= 1) ? 0 : frame + 1;
  lcd.firstPage();

  do {
    //drawMillingScreen();
    //drawLogScreen();
  } while (lcd.nextPage());
}

void UI::drawLogScreen() {
  lcd.setFont(u8g2_font_5x8_mr);
  lcd.setColorIndex(1);
  lcd.drawLog(0, 7, state.logger);
}

void UI::drawMillingScreen() {
  lcd.setFont(u8g2_font_5x8_mr);
  lcd.setColorIndex(1);

  float progress = 0;
  String progressStr;
  int progressBar;
  String timeStr = formatTime(state.milling.elapsedSeconds);

  if (state.milling.totalLines > 0) {
    progress = (float) state.milling.currentLine / state.milling.totalLines;
  }
  if (progress > 1) {
    progress = 1;
  }

  progressStr += (int) round(progress * 100);
  progressStr += "%";
  progressBar = 122 * progress;

  lcd.drawStr(0, 8, state.milling.filename.c_str());
  lcd.drawRFrame(0, 10, 128, 10, 3);
  lcd.drawRBox(1, 11, 4 + progressBar, 8, 2); // 5->127 (122 steps)
  lcd.drawStr(0, 28, progressStr.c_str());
  lcd.drawStr(128 - (5 * timeStr.length()), 28, timeStr.c_str());

  //lcd.drawLog(0, 38, state.logger);

  drawTextButton(0, "Load");
  drawPlayButton(1);
  drawPauseButton(2);
  drawStopButton(3);
}

void UI::firstPage() {
  lcd.firstPage();
}

uint8_t UI::nextPage() {
  return lcd.nextPage();
}

void UI::setFont(const uint8_t *font, byte colorIndex) {
  lcd.setFont(font);
  lcd.setColorIndex(colorIndex);
}

void UI::drawStr(u8g2_uint_t x, u8g2_uint_t y, String str) {
  lcd.drawStr(x, y, str.c_str());
}

void UI::drawPickList(u8g2_uint_t y, byte size, String items[], byte itemsCount, int selectedIndex) {
  lcd.setFont(u8g2_font_6x10_mr);
  lcd.setColorIndex(1);

  byte offset = 0;
  if (size >= itemsCount) {
    size = itemsCount;
  }
  else {
    byte centerIndex = floor(size / 2.0);
    if (selectedIndex > centerIndex) {
      offset = selectedIndex - centerIndex;
    }
    if (offset + size > itemsCount) {
      offset = itemsCount - size;
    }
  }

  for (int i = 0; i < size; i++) {
    if (i + offset == selectedIndex) {
      lcd.drawBox(0, i * 11, 128, 10);
      lcd.setColorIndex(0);
      drawStr(1, i * 11 + 8, items[i + offset]);
      lcd.setColorIndex(1);
    }
    else {
      drawStr(1, i * 11 + 8, items[i + offset]);
    }
  }
}

void UI::drawProgressBar(int16_t y, float progress) {
  int progressBar = 122 * progress;
  lcd.drawRFrame(0, y, 128, 10, 3);
  lcd.drawRBox(1, y + 1, 4 + progressBar, 8, 2); // 5->127 (122 steps)
}

void UI::drawPlayButton(byte pos) {
  int16_t x = calculateButtonLeft(pos);
  int16_t width = calculateButtonWidth(pos);

  lcd.setColorIndex(1);
  lcd.drawRBox(x, 52, width, 13, 2);

  lcd.setColorIndex(0);
  lcd.drawTriangle(x + 14, 54, x + 14, 62, x + 19, 58);
}

void UI::drawStopButton(byte pos) {
  int16_t x = calculateButtonLeft(pos);
  int16_t width = calculateButtonWidth(pos);

  drawButton(x, width);
  lcd.setColorIndex(0);
  lcd.drawBox(x + 13, 55, 6, 6);
}

void UI::drawPauseButton(byte pos) {
  int16_t x = calculateButtonLeft(pos);
  int16_t width = calculateButtonWidth(pos);

  drawButton(x, width);
  lcd.setColorIndex(0);
  lcd.drawBox(x + 14, 55, 2, 6);
  lcd.drawBox(x + 17, 55, 2, 6);
}

void UI::drawTextButton(byte pos, String label) {
  int16_t x = calculateButtonLeft(pos);
  int16_t width = calculateButtonWidth(pos);
  byte textX = x + round((width - label.length() * 5) / 2.0);

  drawButton(x, width);
  lcd.setColorIndex(0);
  lcd.setFont(u8g2_font_5x8_mr);
  lcd.drawStr(textX, 61, label.c_str());
}

void UI::drawButton(int16_t x, int16_t width) {
  lcd.setColorIndex(1);
  lcd.drawRBox(x, 52, width, 13, 2);
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
