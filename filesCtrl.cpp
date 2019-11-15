#include "filesCtrl.h"

#define STATE_INIT 0
#define STATE_READING 1
#define STATE_ERROR 2

FilesCtrl::FilesCtrl(UI *ui, KeyPad *keyPad, SPIClass *spi, int csPin) 
    : _ui(*ui), _keyPad(*keyPad), _spi(*spi), _csPin(csPin) {
}

void FilesCtrl::setup() {
  pinMode(_csPin, OUTPUT);
}

void FilesCtrl::start() {
  _sdCard = false;
  _loaded = false;
  _folderName = "/";
  
  Serial.print("Initializing SD cad .. ");
  if (!SD.begin(_csPin, _spi)) {
    Serial.println("error");
  }
  else {
    Serial.println("OK");

    _filesCount = 0;
    _selectedFileIndex = 0;
    _folder = SD.open(_folderName, FILE_READ);
    if (!_folder) {
      Serial.println("Failed to open root");
    }
    else {
      Serial.println("Reading root");
      _sdCard = true;
    }
  }
}

void FilesCtrl::stop() {
  _folder.close();
}

void FilesCtrl::update(int deltaMs) {
  if (_sdCard && _loaded) {
    _pickingFile();
  }
  else if (_sdCard) {
    _loadingFolder();  
  }
  else {
    _showError("No SD card");
  }
}

void FilesCtrl::_pickingFile() {
  if (_keyPad.isKeyPressed(KEYCODE_ZDOWN)) {
    if (_selectedFileIndex + 1 < _filesCount) {
      _selectedFileIndex ++;
      _showFiles();
    }
  }
  else if (_keyPad.isKeyPressed(KEYCODE_ZUP)) {
    if (_selectedFileIndex > 0) {
      _selectedFileIndex --;
      _showFiles();
    }
  }
  else if (_keyPad.isKeyPressed(KEYCODE_D)) { // select
    
  }
  else if (_keyPad.isKeyPressed(KEYCODE_A)) { // back
    
  }
}

void FilesCtrl::_loadingFolder() {
  String filename;
  File entry = _folder.openNextFile();

  if (!entry || _filesCount >= MAX_FILES) {
    Serial.println("Done reading root");
    _folder.close();
    _loaded = true;
    _showFiles();
  }
  else if (!entry.isDirectory()) {
    filename = entry.name();
    filename = filename.substring(_folderName.length());
    Serial.print(filename);
    Serial.print(" ");
    Serial.print(entry.size() / 1024, DEC);
    Serial.println(F(" kB"));

    _files[_filesCount] = filename;
    _filesCount ++;
    _showLoading((float) _filesCount / MAX_FILES);
  }
  entry.close();
}

void FilesCtrl::_showError(String msg) {
  _ui.firstPage();
  do {
    _ui.setFont(u8g2_font_5x8_mr);
    _ui.drawStr(0, 7, msg);
  } while (_ui.nextPage());
}

void FilesCtrl::_showLoading(float progress) {
  _ui.firstPage();
  do {
    _ui.setFont(u8g2_font_5x8_mr);
    _ui.drawStr(0, 7, "Reading SD card");
    _ui.drawProgressBar(9, progress);
  } while (_ui.nextPage());
}

void FilesCtrl::_showFiles() {
  _ui.firstPage();
  do {
    if (_filesCount == 0) {
      _ui.drawStr(0, 7, "No file");
    }
    _ui.drawPickList(0, 4, _files, _filesCount, _selectedFileIndex);
    _ui.drawTextButton(0, "Back");
    _ui.drawPlayButton(3);
  } while (_ui.nextPage());
}

/*
void selectFile() {
  boolean found = false;

  File dir = SD.open("/");
  if (!dir) {
    state.logger.println(F("Card Read Error"));
    return;
  }

  while (!found) {
    File entry = dir.openNextFile();
    if (!entry) {
      state.logger.println(F("No file found"));
      break; // no more files
    }
    if (!entry.isDirectory()) {
      state.logger.println(entry.name());
      state.logger.print(entry.size() / 1024, DEC);
      state.logger.println(F(" kB"));

      file = SD.open(entry.name());
      if (!file.available()) {
        state.logger.println(F("Cannot open file"));
        return;
      }

      found = true;
      mode = MODE_IDLE;
      state.milling.filename = entry.name();
      state.milling.totalLines = 0;
      state.milling.currentLine = 0;
    }
    entry.close();
  }
  dir.close();
}
*/
