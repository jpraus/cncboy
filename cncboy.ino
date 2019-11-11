#include <SD.h>
#include <U8g2lib.h>
#include "ui.h"
#include "state.h"
#include "keypad.h"

#define SD_CS 21
#define DSPY_CS 5
#define DSPY_BCKLIGHT 2 // TODO
#define RXD2 16
#define TXD2 17

#define KEY_FUNCTION 36 // VP
#define KEY_X 39 // VN
#define KEY_Y 34
#define KEY_Z 35

//U8G2_ST7920_128X64_1_SW_SPI lcd(U8G2_R0, /* CLK=*/ 18, /* MOSI=*/ 23, /* CS=*/ 5);
//U8G2_ST7920_128X64_1_SW_SPI lcd(U8G2_R0, /* CLK=*/ DSPY_EN, /* MOSI=*/ DSPY_RW, /* CS=*/ DSPY_RS, /* reset=*/ U8X8_PIN_NONE);
U8G2_ST7920_128X64_F_HW_SPI u8g2(U8G2_R0, /* CS=*/ DSPY_CS);

STATE state;
UI ui(&u8g2, &state, DSPY_BCKLIGHT);
KeyPad keypad(KEY_FUNCTION, KEY_X, KEY_Y, KEY_Z);

File file;

int everySecondTimer = 0;
unsigned long deltaMsRef = 0;

// mode
#define MODE_INIT 0
#define MODE_SELECT_FILE 1
#define MODE_LOAD_FILE 2
#define MODE_SEND_COMMAND 3
#define MODE_AWAIT_ACK 4
#define MODE_DONE 5
#define MODE_ERROR 99

byte mode = MODE_INIT;

void setup() {
  Serial.begin(115200); // USB
  Serial.setDebugOutput(0);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // CNC

  // modules
  keypad.setup();
  ui.setup();

  mode = MODE_INIT;
  state.milling.elapsedSeconds = 0;
}

void loop() {
  int deltaMs = getDeltaMs();
  keypad.update();

  // do every 1 second
  everySecondTimer -= deltaMs;
  if (everySecondTimer <= 0) {
    everySecondTimer += 1000;
    if (mode == MODE_SEND_COMMAND || mode == MODE_AWAIT_ACK) { // milling
      state.milling.elapsedSeconds ++;
    }
    ui.update();
  }

  switch (mode) {
    case MODE_INIT:
      initialize();
      break;
    case MODE_SELECT_FILE:
      selectFile();
      break;
    case MODE_LOAD_FILE:
      loadFile();
      break;
    case MODE_SEND_COMMAND:
      streamFile();
      ui.update();
      break;
    case MODE_AWAIT_ACK:
      receiveReponse();
      break;
  }
}

void initialize() {
  state.logger.println(F("SD card .."));
  pinMode(SD_CS, OUTPUT);

  if (!SD.begin(SD_CS)) {
    mode = MODE_ERROR;
    state.logger.println(F("Card Error"));
  }
  else {
    mode = MODE_SELECT_FILE;
    state.logger.println(F("Card Found"));
  }

  deltaMsRef = millis();
  everySecondTimer = 1000;
  delay(1000);
}

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
      state.logger.print(entry.size()/1024, DEC);
      state.logger.println(F(" kB"));

      file = SD.open(entry.name());
      if (!file.available()) {
        state.logger.println(F("Cannot open file"));
        return;
      }

      found = true;
      mode = MODE_LOAD_FILE;
      state.milling.filename = entry.name();
      state.milling.totalLines = 0;
      state.milling.currentLine = 0;
    }
    entry.close();
  }
  dir.close();
}

void loadFile() {
  String line;

  if (file.available()) {
    line = file.readStringUntil('\n');
    if (isValidCommand(line)) {
      state.milling.totalLines++;  
    }
  }
  else {
    state.logger.print(state.milling.totalLines);
    state.logger.println(F(" lines"));

    file.seek(0); // reset file
    state.milling.elapsedSeconds = 0;
    mode = MODE_SEND_COMMAND;
  }
}

void streamFile() {
  String line;
  
  if (file.available()) {
    line = file.readStringUntil('\n');
    if (isValidCommand(line)) {
      Serial2.print(line);
      Serial2.flush();

      Serial.println(line);
      state.logger.println(line);
      mode = MODE_AWAIT_ACK;

      state.milling.currentLine ++;
      Serial.print(state.milling.currentLine);
      Serial.print(" of ");
      Serial.println(state.milling.totalLines);
      
    }
  }
  else {
    state.logger.println(F("Reading finished"));
    file.close();
    mode = MODE_DONE;
  }
}

boolean isValidCommand(String line) {
  line.trim();
  if (line.length() == 0) {
    return false;
  }
  if (line.charAt(0) == ';' || line.charAt(0) == '%' || line.charAt(0) == '(') { // skip comments
    return false;
  }
  return true;
}

String receiveReponse() {
  String response;

  while (Serial2.available()) {
    response += char(Serial2.read());
    //delay(10);
  }
  if (response.length() > 0) {
    response.trim();
    if (response.indexOf("ok") != -1) {
      //delay(10);
      mode = MODE_SEND_COMMAND;
    }
    else if (response.indexOf("error") != -1) {
      state.logger.println(response);
      mode = MODE_ERROR;
    }
    Serial.println(response);
  }
  return response;
}

/*
int encoder()
{
  mode = (digitalRead(9) << 1) | digitalRead(8);
  if (mode != prevmode) {
    if (mode == nextEncoderState[prevmode]) {
       beep();
       return 1;
    } else if (mode == prevEncoderState[prevmode]) {
       beep(); 
       return 0;
    }
    prevmode = mode;
  }
}

void beep()
{
  digitalWrite(A1, HIGH);
  delay(2);
  digitalWrite(A1, LOW);
}

void leggisd()
{
  File root = SD.open("/");
  if (!root) {
    logMessage("SD Card Error");
    delay(1000);
    return;
  }

  int i = 0;
  int j = 1;
  //lcd.clear();
  //while(true)
  //{
    while (i != j)
    {
      File entry =  root.openNextFile();
      if (!entry)
      {
        root.rewindDirectory();
        i = 0;
        j = 1;
        break;
      }
      if (entry.isDirectory())
      {
        delay(10);
      }
      else
      {
        i++;
        myfile = entry.name();

        state.logger.print("> ");
        state.logger.print(myfile);
        state.logger.print("\n");

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(j);
        lcd.print(">");
        lcd.print(myfile);
      }
      entry.close();
    }
    /*if ( encoder() == 0)
    {
      if (j > 1)
      {
        j--;
        delay(500);
      }
    }
    if ( encoder() == 1)
    {
      j++;
      delay(500);
    }
    digitalWrite(pinenc, HIGH);
    if ( digitalRead(pinenc) == LOW)
    {
      beep();
      delay(10);
      root.close();
      stream(myfile);
      break;
    }
  //}
  root.close();
}

void stream(char *myfile)
{
  int nstrm = 0;
  int nstr = 0;
  File strFile = SD.open(myfile);

  while (!strFile.available())
  {
    delay(1000);
    return;
  }
  
  while (strFile.available())
  {
    gcd = strFile.readStringUntil('\n');
    nstr++;
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Stream!!!");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Avvio:");
  lcd.setCursor(7, 0);
  lcd.print(myfile);
  delay(1000);
  strFile.seek(0);
  while (strFile.available())
  {
    gcd = strFile.readStringUntil('\n');
    gcd = gcd.substring(0, gcd.length() - 1);
    Serial.print(gcd);
    Serial.write(0x0A); // seguita solo da /n (LF)
    Serial.flush();
    nstrm++;
    lcd.setCursor(0, 1);
    lcd.print(nstr);
    lcd.print("/");
    lcd.print(nstrm);
    lcd.print(" ");
    lcd.setCursor(0, 2);
    lcd.print(gcd);
    while(RxString.lastIndexOf('ok') <= 0)
    {
      serialread();
      if ( RxString.lastIndexOf('error') > 0)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(RxString);
        lcd.setCursor(0, 1);
        lcd.print(words[3]);
        RxString = "";
        strFile.close();
        delay(1000);
        break;
      }
    }
    RxString = "";
    //readpos();
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(words[5]);
  delay(2000);
  strFile.close();
  return;
}
*/

int getDeltaMs() {
  unsigned long time = millis();
  if (time < deltaMsRef) {
    deltaMsRef = time; // overflow of millis happen, we lost some millis but that does not matter for us, keep moving on
    return time;
  }
  int deltaMs = time - deltaMsRef;
  deltaMsRef += deltaMs;
  return deltaMs;
}
