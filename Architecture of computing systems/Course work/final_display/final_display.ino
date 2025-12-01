#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int trigPin1 = 3;
const int echoPin1 = 4;
const int trigPin2 = 5; 
const int echoPin2 = 6;
const int relayPin = 2;

const int maxDistance = 15;
const unsigned long PUMP_DURATION = 2000; 

bool pumpOn = false;
unsigned long pumpStartTime = 0;

bool sensor1OK = false;
bool sensor2OK = false;

uint8_t sensor1BadCount = 0;
uint8_t sensor2BadCount = 0;
uint8_t sensor1GoodCount = 0;
uint8_t sensor2GoodCount = 0;

bool sensorErrorEver = false;
bool sensorsEverAllOK = false;

const uint8_t BAD_THRESHOLD  = 10;
const uint8_t GOOD_THRESHOLD = 3;

void showLines(const String &l1 = "",
               const String &l2 = "",
               const String &l3 = "",
               const String &l4 = "") 
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(l1);
  if (l2.length()) display.println(l2);
  if (l3.length()) display.println(l3);
  if (l4.length()) display.println(l4);
  display.display();
}

void showStatus(long dist1, long dist2, bool pumpOn) {
  display.clearDisplay();
  display.setCursor(0, 0);

  display.println("RUN MODE");

  display.print("S1: ");
  display.print(dist1);
  display.println(" cm");

  display.print("S2: ");
  display.print(dist2);
  display.println(" cm");

  display.print("Pump: ");
  display.println(pumpOn ? "ON" : "OFF");

  if (sensorErrorEver) {
    display.println("Reboot after error");
  }

  display.display();
}

void showSensorError(long dist1, long dist2) {
  String line2;

  if (!sensor1OK && !sensor2OK) {
    line2 = "Sensor 1 & 2 error";
  } else if (!sensor1OK) {
    line2 = "Sensor 1 error";
  } else if (!sensor2OK) {
    line2 = "Sensor 2 error";
  }

  String line3 = "S1:" + String(dist1) + " S2:" + String(dist2);
  showLines("SENSOR ERROR", line2, line3, "Reconnect & reboot");
}

void testSensorPins() {
  showLines("--- D5-D6 TEST ---", "D5 (Trig): HIGH");

  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  delay(100);
  digitalWrite(5, LOW);
  showLines("--- D5-D6 TEST ---", "D5 (Trig): LOW");
  delay(500);

  pinMode(6, INPUT);
  int echoState = digitalRead(6);

  showLines("--- D5-D6 TEST ---",
            "D6 (Echo) state:",
            String(echoState),
            "--- TEST DONE ---");
  delay(1000);
}

long readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 20000);
  if (duration == 0) return 999;

  long distance = duration * 0.034 / 2;

  if (distance > 400) return 999;

  return distance;
}

void updateSensorHealth(long dist1, long dist2) {
  bool s1GoodNow = (dist1 != 999 && dist1 > 0 && dist1 < 400);
  bool s2GoodNow = (dist2 != 999 && dist2 > 0 && dist2 < 400);

  if (s1GoodNow) {
    if (sensor1GoodCount < GOOD_THRESHOLD) sensor1GoodCount++;
    sensor1BadCount = 0;
  } else {
    if (sensor1BadCount < BAD_THRESHOLD) sensor1BadCount++;
    sensor1GoodCount = 0;
  }

  if (sensor1GoodCount >= GOOD_THRESHOLD) sensor1OK = true;
  if (sensor1BadCount >= BAD_THRESHOLD)   sensor1OK = false;

  if (s2GoodNow) {
    if (sensor2GoodCount < GOOD_THRESHOLD) sensor2GoodCount++;
    sensor2BadCount = 0;
  } else {
    if (sensor2BadCount < BAD_THRESHOLD) sensor2BadCount++;
    sensor2GoodCount = 0;
  }

  if (sensor2GoodCount >= GOOD_THRESHOLD) sensor2OK = true;
  if (sensor2BadCount >= BAD_THRESHOLD)   sensor2OK = false;
}

void setup() {
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(relayPin, HIGH);
  pumpOn = false;

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;) {}
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  showLines("=== SYSTEM STARTED ===", "Diag: D5-D6 test");
  delay(1000);

  testSensorPins();
  showLines("WAITING SENSORS", "Connect HC-SR04");
  delay(1000);
}

void loop() {
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 1000) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    lastBlink = millis();
  }

  long dist1 = readDistance(trigPin1, echoPin1);
  long dist2 = readDistance(trigPin2, echoPin2);

  updateSensorHealth(dist1, dist2);

  bool allSensorsOK = sensor1OK && sensor2OK;

  if (allSensorsOK) {
    sensorsEverAllOK = true;
  }

  if (!allSensorsOK) {
    if (sensorsEverAllOK) {
      sensorErrorEver = true;
    }

    if (pumpOn) {
      digitalWrite(relayPin, HIGH);
      pumpOn = false;
    }
    showSensorError(dist1, dist2);
    delay(100);
    return;
  }

  bool hand1 = (dist1 > 0 && dist1 <= maxDistance);
  bool hand2 = (dist2 > 0 && dist2 <= maxDistance);

  if (pumpOn) {
    if (!hand1 && !hand2) {
      digitalWrite(relayPin, HIGH);
      pumpOn = false;
      showLines("ACTIVATION", "CANCELLED", "Pump OFF");
      delay(200);
    }
    else if (millis() - pumpStartTime >= PUMP_DURATION) {
      digitalWrite(relayPin, HIGH);
      pumpOn = false;
      showLines("Pump OFF", "2 seconds passed");
      delay(200);
    }
  } else {
    if (hand1 && hand2) {
      digitalWrite(relayPin, LOW);
      pumpOn = true;
      pumpStartTime = millis();
      showLines("BOTH HANDS", "Pump ON", "for 2 sec");
      delay(150);
    }
  }

  showStatus(dist1, dist2, pumpOn);

  delay(20);
}
