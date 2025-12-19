#include <Arduino.h>
#include <TFT_eSPI.h>
#include <DFRobotDFPlayerMini.h>

// ================= PINES =================
#define BTN_PLAY   25
#define BTN_NEXT   32
#define BTN_PREV   33
#define POT_PIN    26
#define BUSY_PIN   27

#define DF_RX 16
#define DF_TX 17

// ================= OBJETOS =================
TFT_eSPI tft = TFT_eSPI();
HardwareSerial dfSerial(2);
DFRobotDFPlayerMini player;

// ================= ESTADO =================
bool isPlaying = false;
bool pausedManually = false;
bool lastBusy = HIGH;

int lastVolume = -1;
unsigned long lastVolChange = 0;

String statusMsg = "";
unsigned long msgUntil = 0;

// ================= UI =================
void showStatus(const String &msg, unsigned long duration = 1500) {
  statusMsg = msg;
  msgUntil = millis() + duration;
}

void drawUI() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 40);

  if (millis() < msgUntil && statusMsg != "") {
    tft.println(statusMsg);
  } else {
    tft.println("MP3 Player");
    tft.setCursor(20, 70);
    tft.println(isPlaying ? "PLAYING" : "STOP");
  }
}

// ================= SETUP =================
void setup() {
  pinMode(BTN_PLAY, INPUT_PULLUP);
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BUSY_PIN, INPUT);

  analogReadResolution(12);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 40);
  tft.println("Leyendo SD...");

  dfSerial.begin(9600, SERIAL_8N1, DF_RX, DF_TX);
  delay(1500);

  if (!player.begin(dfSerial)) {
    tft.fillScreen(TFT_BLACK);
    tft.println("DF ERROR");
    while (1);
  }

  int vol = map(analogRead(POT_PIN), 0, 4095, 0, 30);
  player.volume(vol);
  lastVolume = vol;

  player.play(1);
  isPlaying = true;
  pausedManually = false;

  drawUI();
}

// ================= LOOP =================
void loop() {
  // ===== BUSY sincroniza estado real =====
  bool busy = digitalRead(BUSY_PIN);
  if (busy != lastBusy) {
    if (busy == LOW) {
      isPlaying = true;
    } else {
      if (!pausedManually) isPlaying = false;
    }
    drawUI();
    lastBusy = busy;
  }

  // ===== PLAY / STOP =====
  static bool lastPlayBtn = HIGH;
  bool playBtn = digitalRead(BTN_PLAY);

  if (lastPlayBtn == HIGH && playBtn == LOW) {
    if (isPlaying) {
      player.pause();
      pausedManually = true;
      isPlaying = false;
      showStatus("STOP");
    } else {
      player.start();
      pausedManually = false;
      isPlaying = true;
      showStatus("PLAY");
    }
    drawUI();
    delay(200);
  }
  lastPlayBtn = playBtn;

  // ===== NEXT / PREV (solo UI) =====
  static bool lastNext = HIGH, lastPrev = HIGH;
  bool next = digitalRead(BTN_NEXT);
  bool prev = digitalRead(BTN_PREV);

  if (lastNext == HIGH && next == LOW) {
    showStatus("NEXT");
    drawUI();
  }
  if (lastPrev == HIGH && prev == LOW) {
    showStatus("PREV");
    drawUI();
  }

  lastNext = next;
  lastPrev = prev;

  // ===== VOLUMEN (limitado) =====
  int pot = analogRead(POT_PIN);
  int vol = map(pot, 0, 4095, 0, 30);

  if (vol != lastVolume && millis() - lastVolChange > 200) {
    player.volume(vol);
    lastVolume = vol;
    lastVolChange = millis();
  }

  delay(30);
}
