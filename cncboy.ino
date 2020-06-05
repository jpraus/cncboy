#include <SD.h>
#include <U8g2lib.h>
#include "ui.h"
#include "keypad.h"
#include "millingCtrl.h"
#include "filesCtrl.h"
#include "grbl.h"

#define SD_CS 21
#define DSPY_CS 5
#define DSPY_BCKLIGHT 2 // TODO
#define RXD2 16
#define TXD2 17

#define KEY_FUNCTION 36 // VP
#define KEY_X 39 // VN
#define KEY_Y 34
#define KEY_Z 35

#define HSPI_MISO 15
#define HSPI_MOSI 13
#define HSPI_CLK 14

U8G2_ST7920_128X64_F_HW_SPI u8g2(U8G2_R0, /* CS=*/ DSPY_CS); // VSPI for display
SPIClass SDSPI(HSPI); // HSPI for SD card

Grbl grbl;
UI ui(&u8g2, DSPY_BCKLIGHT);
KeyPad keypad(KEY_FUNCTION, KEY_X, KEY_Y, KEY_Z);

MillingCtrl millingCtrl(&grbl, &ui, &keypad);
FilesCtrl filesCtrl(&ui, &keypad);

// screen
#define SCREEN_HOME 0
#define SCREEN_FILES 1
#define SCREEN_MILLING 2
#define SCREEN_CALIBRATION 3

byte screen = SCREEN_FILES;

void setup() {
  Serial.begin(115200); // USB
  Serial.setDebugOutput(0);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // CNC

  // HSPI for SD card
  SDSPI.begin(HSPI_CLK, HSPI_MISO, HSPI_MOSI, -1);

  // modules
  keypad.setup();
  ui.setup();
  filesCtrl.setup();

  // initial delay to warm everything up
  delay(200);

  // init SD card
  pinMode(SD_CS, OUTPUT);
  Serial.println("Initializing SD card .. ");
  if (!SD.begin(SD_CS, SDSPI)) {
    Serial.println("No SD Card");
  }

  // init GRBL interface
  grbl.start();

  // init screen
  screen = SCREEN_FILES;
  filesCtrl.start();
}

void loop() {
  unsigned long nowMillis = millis();
  byte result;

  keypad.update(nowMillis);

  switch (screen) {
    case SCREEN_HOME:
      break;

    case SCREEN_FILES:
      // screen where you can selected files to start milling
      result = filesCtrl.update();
      switch (result) {
        case FILES_RESULT_SELECTED:
          millingCtrl.start(filesCtrl.getSelectedFile());
          screen = SCREEN_MILLING;
          break;
        case FILES_RESULT_BACK:
          //screen = SCREEN_HOME;
          break;
      }
      break;

    case SCREEN_MILLING:
      // running milling program
      result = millingCtrl.update(nowMillis);
      switch (result) {
        case MILLING_RESULT_BACK:
          millingCtrl.stop();
          filesCtrl.start();
          screen = SCREEN_FILES;
          break;
      }
      break;
  }

  delay(1);
}
