#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <TFT_eSPI.h>

// =====================
// PINES
// =====================
#define BTN_PLAY 32
#define BTN_NEXT 33
#define BTN_PREV 25
#define POT_VOL  34

#define DF_TX 26
#define DF_RX 27

// ===================== OBJETOS
HardwareSerial dfSerial(2);
DFRobotDFPlayerMini dfplayer;
TFT_eSPI tft = TFT_eSPI();

// ===================== VARIABLES
bool df_ok = false;
bool tft_ok = false;
bool isPlaying = false;
int currentTrack = 1;
int lastVolume = -1;
String statusText = "BOOT";

// ===================== PROTOTIPOS
void handleButtons();
void handleVolume();
void showStatus(const String &msg);




// ===================== PROGRAMA PRINCIPAL
void setup() {
  Serial.begin(115200);

  pinMode(BTN_PLAY, INPUT_PULLUP);
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_PREV, INPUT_PULLUP);

  // ----- Pantalla -----
  tft.init();
  tft.setRotation(1);
  tft_ok = true;

  showStatus("BOOT");

  // ----- DFPlayer -----
  dfSerial.begin(9600, SERIAL_8N1, DF_TX, DF_RX);

  if (dfplayer.begin(dfSerial)) {
    df_ok = true;
    dfplayer.volume(15);
    dfplayer.play(currentTrack);
    isPlaying = true;
    showStatus("Playing");
  } else {
    showStatus("DFPlayer OFF");
  }
}

// ===================== PROGRAMA PRINCIPAL
void loop() {
  handleButtons();
  handleVolume();
}

// ===================== DEFINICION PULSADORES
void handleButtons() {
  static unsigned long lastTime = 0;
  if (millis() - lastTime < 200) return;

  if (!digitalRead(BTN_PLAY)) {
    if (df_ok) {
      if (isPlaying) {
        dfplayer.pause();
        showStatus("Stop");
      } else {
        dfplayer.start();
        showStatus("Playing");
      }
      isPlaying = !isPlaying;
    }
    lastTime = millis();
  }

  if (!digitalRead(BTN_NEXT)) {
    currentTrack++;
    if (df_ok) dfplayer.play(currentTrack);
    showStatus("Next");
    isPlaying = true;
    lastTime = millis();
  }

  if (!digitalRead(BTN_PREV)) {
    if (currentTrack > 1) currentTrack--;
    if (df_ok) dfplayer.play(currentTrack);
    showStatus("Prev");
    isPlaying = true;
    lastTime = millis();
  }
}

// ===================== DEFINICION POTENCIOMETRO
void handleVolume() {
  int adc = analogRead(POT_VOL);
  int vol = map(adc, 0, 4095, 0, 30);

  if (abs(vol - lastVolume) > 1) {
    lastVolume = vol;
    if (df_ok) dfplayer.volume(vol);
    showStatus("Vol: " + String(vol));
    delay(50);
  }
}

// ===================== DEFINICION MOSTRAR PANTALLA
void showStatus(const String &msg) {
  statusText = msg;

  Serial.println(msg);

  if (tft_ok) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("MP3 PLAYER", 20, 10, 2);
    tft.drawString(msg, 20, 60, 2);
    tft.drawString("Track: " + String(currentTrack), 20, 90, 2);
    tft.drawString("Vol: " + String(lastVolume), 20, 120, 2);
  }
}


