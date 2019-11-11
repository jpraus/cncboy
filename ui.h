#ifndef UI_H
#define UI_H

#include "Arduino.h"
#include <U8g2lib.h>
#include "state.h"

class UI {
  public:
    UI(U8G2 *lcd, STATE *state, byte backlightPin);
    void setup();
    void update();

  private:
    U8G2 &_lcd;
    STATE &_state;
    byte _backlightPin;
    byte _frame = 0;

    void drawLogScreen();
    void drawMillingScreen();

    void drawStopButton(byte pos);
    void drawPlayButton(byte pos);
    void drawPauseButton(byte pos);
    void drawButton(int16_t x, int16_t width);
    void drawTextButton(byte pos, String label);
    int16_t calculateButtonLeft(byte pos);
    int16_t calculateButtonWidth(byte pos);
    String formatTime(unsigned int seconds);
};

#endif
