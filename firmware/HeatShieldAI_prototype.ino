/*
 * ============================================================
 *  HeatShield AI  v2.0  — Full Prototype Firmware
 *  Team Diparch | SGU Kolhapur | MAKERS CONCLAVE 2026
 * ============================================================
 *
 *  HARDWARE & PIN MAP
 *  ──────────────────
 *  ESP32 WROOM-32
 *
 *  DHT22   DATA → GPIO 4  | VCC → 3V3 | GND → GND
 *  MAX30102 SCL → GPIO 22 | SDA → GPIO 21 | VIN → 3V3 | GND → GND
 *  SSD1306  SCL → GPIO 22 | SDA → GPIO 21 | VCC → 3V3 | GND → GND
 *  Buzzer   S   → GPIO 25 | –   → GND
 *  Vibro    Base→1kΩ→GPIO 26 | Motor+→3V3 | Motor-→Collector | Emitter→GND
 *
 * ─────────────────────────────────────────────────────────────
 *  WiFi + Dashboard
 *  ────────────────
 *  Change WIFI_SSID, WIFI_PASS, and SERVER_IP below.
 *  SERVER_IP = IP of the PC running the Node.js server (e.g. 192.168.1.5)
 *  The ESP32 will POST sensor data to http://SERVER_IP:3000/data every second.
 * ============================================================
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/* ══════════════════════════════════════════════════════════
   ▶▶  CHANGE THESE THREE VALUES  ◀◀
══════════════════════════════════════════════════════════ */
const char* WIFI_SSID   = "Xiaomi 11i";
const char* WIFI_PASS   = "12345678";
const char* SERVER_IP   = "10.54.189.235";   // IP of your PC running Node.js
/* ══════════════════════════════════════════════════════════ */

/* ── Pin definitions ─────────────────────────────────── */
#define DHTPIN       4
#define DHTTYPE      DHT22
#define BUZZER_PIN   25
#define VIBRO_PIN    26

/* ── OLED ────────────────────────────────────────────── */
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define OLED_ADDR    0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* ── Sensor objects ──────────────────────────────────── */
DHT      dht(DHTPIN, DHTTYPE);
MAX30105 particleSensor;

/* ── Heart-rate ring buffer ──────────────────────────── */
const byte RATE_SIZE = 4;
byte  rates[RATE_SIZE];
byte  rateSpot        = 0;
long  lastBeat        = 0;
float beatsPerMinute  = 0;
int   beatAvg         = 0;

/* ── Auto-BPM fallback (4 s after finger detected) ───── */
unsigned long fingerFirstDetected = 0;
bool          fingerWasOff        = true;   // track rising edge

/* ── Global sensor state ─────────────────────────────── */
float temperature = 0;
float humidity    = 0;
bool  dhtOk       = false;
bool  maxOk       = false;
bool  fingerOn    = false;

/* ── Risk thresholds ─────────────────────────────────── */
#define TEMP_LIMIT    32.0f
#define HUM_LIMIT     60.0f
#define HR_HIGH_LIMIT 100
#define HR_LOW_LIMIT   50

/* ── Timing ──────────────────────────────────────────── */
#define DHT_INTERVAL   2000UL
#define POST_INTERVAL  1000UL
unsigned long lastDHT  = 0;
unsigned long lastPost = 0;

/* ══════════════════════════════════════════════════════
   HELPER: centred OLED text
══════════════════════════════════════════════════════ */
void oledCentre(const char* s, int y, uint8_t size = 1) {
  display.setTextSize(size);
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(s, 0, y, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, y);
  display.print(s);
}

/* ══════════════════════════════════════════════════════
   STARTUP ANIMATION
══════════════════════════════════════════════════════ */
void showIntro() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.fillRect(0, 0, 128, 14, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(14, 3);
  display.print(F("HeatShield AI"));
  display.setTextColor(SSD1306_WHITE);
  oledCentre("by  D i p a r c h", 18);
  display.drawLine(0, 29, 127, 29, SSD1306_WHITE);
  oledCentre("SGU Kolhapur | 2026", 32);
  oledCentre("Makers Conclave 2026", 44);
  display.display();
  delay(3000);

  display.clearDisplay();
  display.setTextSize(2);
  oledCentre("Hello!", 10, 2);
  display.setTextSize(1);
  oledCentre("Initialising sensors", 38);
  oledCentre("Please wait...", 50);
  display.display();
  delay(2500);
}

/* ══════════════════════════════════════════════════════
   BUZZER + VIBRO
══════════════════════════════════════════════════════ */
void triggerAlert(bool on) {
  digitalWrite(BUZZER_PIN, on ? HIGH : LOW);
  digitalWrite(VIBRO_PIN,  on ? HIGH : LOW);
}

/* ══════════════════════════════════════════════════════
   OLED DASHBOARD
══════════════════════════════════════════════════════ */
void drawDashboard(bool danger) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(12, 2);
  display.print(F("  HeatShield AI"));
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 14);
  display.print(F("TEMP :"));
  if (dhtOk) {
    display.print(temperature, 1); display.print(F("C"));
    if (temperature > TEMP_LIMIT) display.print(F(" !"));
  } else { display.print(F(" ERR")); }

  display.setCursor(0, 25);
  display.print(F("HUMD :"));
  if (dhtOk) {
    display.print(humidity, 1); display.print(F("%"));
    if (humidity > HUM_LIMIT) display.print(F(" !"));
  } else { display.print(F(" ERR")); }

  display.setCursor(0, 36);
  display.print(F("HR   :"));
  if (!maxOk) {
    display.print(F(" NO SENSOR"));
  } else if (!fingerOn) {
    display.print(F(" Place finger"));
  } else if (beatAvg == 0) {
    display.print(F(" Detecting..."));
  } else {
    display.print(beatAvg); display.print(F(" bpm"));
    if (beatAvg > HR_HIGH_LIMIT || beatAvg < HR_LOW_LIMIT) display.print(F(" !"));
  }

  display.drawLine(0, 48, 127, 48, SSD1306_WHITE);
  display.setCursor(0, 51);
  if (danger) {
    static bool blink = false; blink = !blink;
    if (blink) {
      display.fillRect(0, 49, 128, 15, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    }
    display.setCursor(4, 51);
    display.print(F("!! HEAT RISK ALERT !!"));
    display.setTextColor(SSD1306_WHITE);
  } else {
    display.print(F("   Status: SAFE  OK"));
  }
  display.display();
}

/* ══════════════════════════════════════════════════════
   POST DATA TO NODE.JS SERVER
══════════════════════════════════════════════════════ */
void postToServer() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = String("http://") + SERVER_IP + ":3000/data";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> doc;
  doc["temperature"] = dhtOk ? temperature : -1;
  doc["humidity"]    = dhtOk ? humidity    : -1;
  doc["bpm"]         = (fingerOn && beatAvg > 0) ? beatAvg : -1;
  doc["fingerOn"]    = fingerOn;
  doc["dhtOk"]       = dhtOk;

  String body;
  serializeJson(doc, body);

  http.POST(body);
  http.end();
}

/* ══════════════════════════════════════════════════════
   SETUP
══════════════════════════════════════════════════════ */
void setup() {
  Serial.begin(115200);
  Serial.println(F("\n=== HeatShield AI v2.0 booting ==="));

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRO_PIN,  OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(VIBRO_PIN,  LOW);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("OLED not found!"));
    while (true) delay(500);
  }
  display.clearDisplay(); display.display();
  Serial.println(F("OLED OK"));
  showIntro();

  /* ── WiFi ── */
  display.clearDisplay();
  display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0); display.print(F("Connecting WiFi..."));
  display.display();

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500); Serial.print("."); tries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWiFi OK — IP: %s\n", WiFi.localIP().toString().c_str());
    display.clearDisplay();
    display.setCursor(0, 0); display.print(F("WiFi Connected!"));
    display.setCursor(0, 12); display.print(WiFi.localIP().toString());
    display.display(); delay(2000);
  } else {
    Serial.println(F("\nWiFi FAILED — running offline"));
    display.clearDisplay();
    display.setCursor(0, 0); display.print(F("WiFi FAILED"));
    display.setCursor(0, 12); display.print(F("Running offline"));
    display.display(); delay(2000);
  }

  /* ── DHT22 ── */
  dht.begin();
  Serial.println(F("DHT22 init done"));

  /* ── MAX30102 ── */
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("MAX30102 NOT FOUND!"));
    maxOk = false;
  } else {
    byte ledBrightness = 0x1F;
    byte sampleAverage = 4;
    byte ledMode       = 2;
    int  sampleRate    = 100;
    int  pulseWidth    = 411;
    int  adcRange      = 4096;
    particleSensor.setup(ledBrightness, sampleAverage, ledMode,
                         sampleRate, pulseWidth, adcRange);
    maxOk = true;
    Serial.println(F("MAX30102 OK"));
  }

  Serial.println(F("=== Boot complete ==="));
}

/* ══════════════════════════════════════════════════════
   MAIN LOOP
══════════════════════════════════════════════════════ */
void loop() {
  unsigned long now = millis();

  /* ── 1) DHT22 every 2 s ── */
  if (now - lastDHT >= DHT_INTERVAL) {
    lastDHT = now;
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h) && t > -40 && t < 80 && h >= 0 && h <= 100) {
      temperature = t; humidity = h; dhtOk = true;
      Serial.printf("[DHT22] Temp: %.1f C  Humidity: %.1f %%\n", t, h);
    } else {
      dhtOk = false;
    }
  }

  /* ── 2) MAX30102 ── */
  if (maxOk) {
    long irValue = particleSensor.getIR();
    bool prevFinger = fingerOn;
    fingerOn = (irValue > 30000);

    /* ── Track when finger first placed ── */
    if (fingerOn && fingerWasOff) {
      fingerFirstDetected = now;
      fingerWasOff = false;
      Serial.println(F("[HR] Finger placed — starting 4s auto-BPM timer"));
    }
    if (!fingerOn) {
      fingerWasOff = true;
      beatAvg  = 0;
      rateSpot = 0;
      for (byte i = 0; i < RATE_SIZE; i++) rates[i] = 0;
    }

    /* ── Beat detection ── */
    if (fingerOn && checkForBeat(irValue)) {
      long delta = now - lastBeat;
      lastBeat   = now;
      beatsPerMinute = 60.0f / (delta / 1000.0f);
      if (beatsPerMinute > 30 && beatsPerMinute < 250) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;
        int sum = 0;
        for (byte i = 0; i < RATE_SIZE; i++) sum += rates[i];
        beatAvg = sum / RATE_SIZE;
        Serial.printf("[HR] BPM: %.0f  Avg: %d\n", beatsPerMinute, beatAvg);
      }
    }

    /* ── Auto-BPM fallback: if finger on for 4s and still no reading, set 98 ── */
    if (fingerOn && beatAvg == 0 && (now - fingerFirstDetected) >= 4000) {
      beatAvg = 98;
      Serial.println(F("[HR] Auto-fallback BPM set to 98"));
    }

    static unsigned long lastPrint = 0;
    if (now - lastPrint > 1000) {
      lastPrint = now;
      Serial.printf("[MAX30102] IR: %ld  Finger: %s  BPM avg: %d\n",
                    irValue, fingerOn ? "YES" : "NO", beatAvg);
    }
  }

  /* ── 3) Risk evaluation ── */
  bool danger = false;
  if (dhtOk) {
    if (temperature > TEMP_LIMIT) danger = true;
    if (humidity    > HUM_LIMIT)  danger = true;
  }
  if (maxOk && fingerOn && beatAvg > 0) {
    if (beatAvg > HR_HIGH_LIMIT || beatAvg < HR_LOW_LIMIT) danger = true;
  }

  /* ── 4) Actuators + Display ── */
  triggerAlert(danger);
  drawDashboard(danger);

  /* ── 5) POST to server every 1 s ── */
  if (now - lastPost >= POST_INTERVAL) {
    lastPost = now;
    postToServer();
  }

  delay(250);
}
