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

    void firstPage();
    uint8_t nextPage(); 
    void setFont(const uint8_t *font, byte colorIndex = 1);
    void drawStr(u8g2_uint_t x, u8g2_uint_t y, String str);
    void drawBox(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h);

    void drawPickList(u8g2_uint_t y, byte size, String items[], byte itemsCount, int selectedIndex);
    void drawProgressBar(int16_t y, float progress);
    void drawStopButton(byte pos);
    void drawPlayButton(byte pos);
    void drawPauseButton(byte pos);
    void drawTextButton(byte pos, String label);
    String formatTime(unsigned int seconds);

  private:
    U8G2 &_lcd;
    STATE &_state;
    byte _backlightPin;
    byte _frame = 0;

    void drawLogScreen();
    void drawMillingScreen();

    void drawButton(int16_t x, int16_t width);
    int16_t calculateButtonLeft(byte pos);
    int16_t calculateButtonWidth(byte pos);
};

#endif
