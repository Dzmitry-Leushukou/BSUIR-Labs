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
bool sensor1OK = true;
bool sensor2OK = true;
bool lastSensor1State = false;
bool lastSensor2State = false;
bool lastPumpState = false;

// ===== HELPER FUNCTIONS FOR OLED =====

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

void showStatus(long dist1, bool hand1, long dist2, bool hand2, bool pumpOn) {
  String s1 = "S1: " + String(dist1) + "cm " + (hand1 ? "[ON]" : "[OFF]");
  String s2 = "S2: " + String(dist2) + "cm " + (hand2 ? "[ON]" : "[OFF]");
  String s3 = String("Pump: ") + (pumpOn ? "ON" : "OFF");
  showLines("RUN MODE", s1, s2, s3);
}

// ===== MAIN CODE =====

void setup() {
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(relayPin, HIGH);
  pumpOn = false;

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // If display is not found - freeze
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  showLines("=== SYSTEM STARTED ===", "Diag: D5-D6 test");
  delay(1500);

  testSensorPins();
  delay(1000);
}

void testSensorPins() {
  showLines("--- D5-D6 TEST ---", "D5 (Trig): HIGH");

  // Test pin D5 (Trig)
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  delay(100);
  digitalWrite(5, LOW);
  showLines("--- D5-D6 TEST ---", "D5 (Trig): LOW");
  delay(500);

  // Test pin D6 (Echo) - input
  pinMode(6, INPUT);
  int echoState = digitalRead(6);

  showLines("--- D5-D6 TEST ---",
            "D6 (Echo) state:",
            String(echoState),
            "--- TEST DONE ---");
  delay(1500);
}

bool checkSensor(int trigPin, int echoPin, String sensorName) {
  String header = "Check " + sensorName;

  for (int i = 0; i < 3; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 25000);

    if (duration > 0) {
      long distance = duration * 0.034 / 2;
      if (distance < 500) {
        showLines(header,
                  "OK",
                  "Dist: " + String(distance) + "cm");
        delay(700);
        return true;
      }
    }
    showLines(header, "Attempt " + String(i + 1), "No signal");
    delay(200);
  }

  showLines(header, "NO SIGNAL", "Limited mode");
  delay(1200);
  return false;
}

void checkSensors() {
  sensor1OK = checkSensor(trigPin1, echoPin1, "Sensor 1");
  sensor2OK = checkSensor(trigPin2, echoPin2, "Sensor 2");

  if (!sensor1OK || !sensor2OK) {
    showLines("WARNING!", "Sensor issue", "Limited mode");
    delay(1500);
  }
}

long readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

void loop() {
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 1000) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    lastBlink = millis();
  }

  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 5000) {
    checkSensors();
    lastCheck = millis();
  }

  long dist1 = readDistance(trigPin1, echoPin1);
  long dist2 = readDistance(trigPin2, echoPin2);

  bool hand1 = (dist1 <= maxDistance && dist1 > 0);
  bool hand2 = (dist2 <= maxDistance && dist2 > 0);

  bool atLeastOneSensorWorking = (dist1 < 500 || dist2 < 500);

  if (!atLeastOneSensorWorking) {
    if (pumpOn) {
      digitalWrite(relayPin, HIGH);
      pumpOn = false;
      showLines("ERROR!", "No sensors", "Pump OFF");
      delay(1500);
    }
    showLines("WAITING...", "No active sensors");
    delay(1000);
    return;
  }

  if (pumpOn) {
    // Pump is running
    if (!hand1 && !hand2) {
      digitalWrite(relayPin, HIGH);
      pumpOn = false;
      showLines("ACTIVATION", "CANCELLED", "Pump OFF");
      delay(1000);
    }
    else if (millis() - pumpStartTime >= PUMP_DURATION) {
      digitalWrite(relayPin, HIGH);
      pumpOn = false;
      showLines("Pump OFF", "2 seconds passed");
      delay(800);
    }
  }
  else if (hand1 && hand2) {
    // Both sensors active -> start pump
    digitalWrite(relayPin, LOW);
    pumpOn = true;
    pumpStartTime = millis();
    showLines("BOTH HANDS", "Pump ON", "for 2 sec");
    delay(500);
  }

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    showStatus(dist1, hand1, dist2, hand2, pumpOn);
    lastPrint = millis();
  }

  delay(100);
}
