#include "filesCtrl.h"

#define STATE_INIT 0
#define STATE_READING 1
#define STATE_ERROR 2

FilesCtrl::FilesCtrl(UI *ui, KeyPad *keyPad) 
    : ui(*ui), keyPad(*keyPad) {
}

void FilesCtrl::setup() {
}

void FilesCtrl::start() {
  if (loaded) {
    stop();
  }

  sdCard = false;
  loaded = false;
  folderName = "/";

  // we expect SD card is already initialized
  filesCount = 0;
  selectedFileIndex = 0;
  folder = SD.open(folderName, FILE_READ);
  if (!folder) {
    Serial.println("Failed to open root");
  }
  else {
    Serial.println("Reading root");
    sdCard = true;
  }
}

void FilesCtrl::stop() {
  folder.close();
}

byte FilesCtrl::update() {
  if (sdCard && loaded) {
    return pickingFile();
  }
  else if (sdCard) {
    loadingFolder();  
  }
  else {
    showError("No SD card");
  }
  return FILES_RESULT_NONE;
}

File FilesCtrl::getSelectedFile() {  
  if (sdCard && loaded) {
    String path = folderName + files[selectedFileIndex];
    File file = SD.open(path);
    if (file.available()) {
      return file;
    }
    Serial.print("Cannot open file ");
    Serial.println(path);
  }
}

byte FilesCtrl::pickingFile() {
  if (keyPad.isKeyPressed(KEYCODE_ZDOWN)) {
    if (selectedFileIndex + 1 < filesCount) {
      selectedFileIndex ++;
      showFiles();
    }
  }
  else if (keyPad.isKeyPressed(KEYCODE_ZUP)) {
    if (selectedFileIndex > 0) {
      selectedFileIndex --;
      showFiles();
    }
  }
  else if (keyPad.isKeyPressed(KEYCODE_D) || keyPad.isKeyPressed(KEYCODE_ZCENTER)) { // select
    return FILES_RESULT_SELECTED;
  }
  else if (keyPad.isKeyPressed(KEYCODE_A)) { // back
    return FILES_RESULT_BACK;
  }
  return FILES_RESULT_NONE;
}

void FilesCtrl::loadingFolder() {
  String filename;
  File entry = folder.openNextFile();

  if (!entry || filesCount >= MAX_FILES) {
    Serial.println("Done reading root");
    folder.close();
    loaded = true;
    showFiles();
  }
  else if (!entry.isDirectory()) {
    filename = entry.name();
    filename = filename.substring(folderName.length());
    Serial.print(filename);
    Serial.print(" ");
    Serial.print(entry.size() / 1024, DEC);
    Serial.println(F(" kB"));

    files[filesCount] = filename;
    filesCount ++;
    showLoading((float) filesCount / MAX_FILES);
  }
  entry.close();
}

void FilesCtrl::showError(String msg) {
  ui.firstPage();
  do {
    ui.setFont(u8g2_font_5x8_mr);
    ui.drawStr(0, 7, msg);
  } while (ui.nextPage());
}

void FilesCtrl::showLoading(float progress) {
  ui.firstPage();
  do {
    ui.setFont(u8g2_font_5x8_mr);
    ui.drawStr(0, 7, "Reading SD card");
    ui.drawProgressBar(9, progress);
  } while (ui.nextPage());
}

void FilesCtrl::showFiles() {
  ui.firstPage();
  do {
    if (filesCount == 0) {
      ui.drawStr(0, 7, "No file");
    }
    ui.drawPickList(0, 4, files, filesCount, selectedFileIndex);
    ui.drawTextButton(0, "Back");
    ui.drawPlayButton(3);
  } while (ui.nextPage());
}
