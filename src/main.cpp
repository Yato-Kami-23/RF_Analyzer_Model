#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_W 128
#define SCREEN_H 64

Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);
RF24 radio(D9, D10); // CE, CSN

const int LED_PIN = D6;
const int NUM_CH = 126; // nRF24 channels 0..125 = 2400..2525 MHz
const int SWEEPS = 50; // sweeps per frame
const int BASE_FRAMES = 3; // frames used to learn the baseline
const float BUSY_RATIO = 0.5; // channel is busy above this share of sweeps
const int WIDE_LIMIT = 30; // alert if this many channels are busy at once
const float NEW_BASE_MAX = 0.05; // channel was quiet in the baseline
const int NEW_MIN_CH = 5; // a new peak needs at least this many channels

int hits[NUM_CH];
float baseline[NUM_CH];
int frameCount = 0;
const char* reason = "LEARN";

void sweepFrame() {
  memset(hits, 0, sizeof(hits));
  for (int s = 0; s < SWEEPS; s++) {
    for (int ch = 0; ch < NUM_CH; ch++) {
      radio.setChannel(ch);
      radio.startListening();
      delayMicroseconds(130);
      radio.stopListening();
      if (radio.testRPD()) hits[ch]++;
    }
  }
}

bool evaluate() {
  int busy = 0, newPeaks = 0;
  for (int ch = 0; ch < NUM_CH; ch++) {
    float r = (float)hits[ch] / SWEEPS;
    if (frameCount < BASE_FRAMES) baseline[ch] += r / BASE_FRAMES;
    if (r >= BUSY_RATIO) {
      busy++;
      if (baseline[ch] <= NEW_BASE_MAX) newPeaks++;
    }
  }
  if (frameCount < BASE_FRAMES) { reason = "LEARN"; return false; }
  if (busy >= WIDE_LIMIT) { reason = "WIDEBAND"; return true; }
  if (newPeaks >= NEW_MIN_CH) { reason = "NEWPEAK"; return true; }
  reason = "OK";
  return false;
}

void drawScreen(bool alert) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("2.4GHz ");
  display.print(alert ? "ALERT " : "");
  display.print(reason);

  for (int ch = 0; ch < NUM_CH; ch++) {
    int h = map(hits[ch], 0, SWEEPS, 0, 40);
    if (h > 0) display.drawFastVLine(ch + 1, 54 - h, h, SSD1306_WHITE);
  }

  display.setCursor(0, 56); display.print("2400");
  display.setCursor(52, 56); display.print("2460");
  display.setCursor(104, 56); display.print("2525");
  display.display();
}

void halt(const char* msg) {
  Serial.println(msg);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(msg);
  display.display();
  while (true) { digitalWrite(LED_PIN, !digitalRead(LED_PIN)); delay(200); }
}

void setup() {
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 3000) { }

  pinMode(LED_PIN, OUTPUT);
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found, check wiring");
  }

  if (!radio.begin()) halt("nRF24 not found");
  radio.setAutoAck(false);
  radio.startListening();
  radio.stopListening();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Learning baseline...");
  display.display();
}

void loop() {
  sweepFrame();
  bool alert = evaluate();
  if (frameCount < 1000) frameCount++;

  Serial.print("SWEEP,"); Serial.print(millis());
  Serial.print(","); Serial.print(alert ? 1 : 0);
  Serial.print(","); Serial.print(SWEEPS);
  Serial.print(","); Serial.print(reason);
  for (int ch = 0; ch < NUM_CH; ch++) { Serial.print(","); Serial.print(hits[ch]); }
  Serial.println();

  digitalWrite(LED_PIN, alert ? HIGH : LOW);
  drawScreen(alert);
}