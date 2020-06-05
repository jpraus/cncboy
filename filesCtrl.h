#ifndef FILES_CTRL_H
#define FILES_CTRL_H

#define MAX_FILES 20

#define FILES_RESULT_NONE 0
#define FILES_RESULT_SELECTED 1
#define FILES_RESULT_BACK 2

#include "Arduino.h"
#include <SD.h>
#include "ui.h"
#include "keypad.h"

class FilesCtrl {
  public:
    FilesCtrl(UI *ui, KeyPad *keyPad);
    void setup();
    void start();
    void stop();
    byte update();
    File getSelectedFile();

  private:
    UI &ui;
    KeyPad &keyPad;

    String folderName;
    File folder;
    String files[MAX_FILES];
    byte filesCount = 0;
    byte selectedFileIndex = 0;

    boolean sdCard = false;
    boolean loaded = false;
    byte state = 0;

    byte pickingFile();
    void loadingFolder();
    void showError(String msg);

    void showLoading(float progress);
    void showFiles();
};

#endif
