#ifndef FILES_CTRL_H
#define FILES_CTRL_H

#define MAX_FILES 20

#include "Arduino.h"
#include <SD.h>
#include "ui.h"
#include "keypad.h"

class FilesCtrl {
  public:
    FilesCtrl(UI *ui, KeyPad *keyPad, SPIClass *spi, int csPin);
    void setup();
    void start();
    void stop();
    void update(int deltaMs);

  private:
    UI &_ui;
    KeyPad &_keyPad;
    SPIClass &_spi;
    int _csPin;

    String _folderName;
    File _folder;
    String _files[MAX_FILES];
    byte _filesCount = 0;
    byte _selectedFileIndex = 0;

    boolean _sdCard = false;
    boolean _loaded = false;
    byte _state = 0;

    void _pickingFile();
    void _loadingFolder();
    void _showError(String msg);

    void _showLoading(float progress);
    void _showFiles();

};

#endif
