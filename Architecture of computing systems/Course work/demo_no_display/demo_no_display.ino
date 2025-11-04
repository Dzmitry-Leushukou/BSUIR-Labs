const int trigPin1 = 3;
const int echoPin1 = 4;
const int trigPin2 = 5; 
const int echoPin2 = 6;
const int relayPin = 2;

const int maxDistance = 15;
const unsigned long PUMP_DURATION = 2000; 
const unsigned long SENSOR_CHECK_INTERVAL = 30000;
bool pumpOn = false;
unsigned long pumpStartTime = 0;
bool sensor1OK = true;
bool sensor2OK = true;
unsigned long lastSensorCheck = 0;

void setup() {
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  digitalWrite(relayPin, HIGH);
  pumpOn = false;
  
  Serial.begin(9600);
  Serial.println("=== СИСТЕМА ЗАПУЩЕНА ===");
  delay(1000); 
  checkSensors();
}

bool checkSensor(int trigPin, int echoPin, String sensorName) {

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
        Serial.println("✅ " + sensorName + " - РАБОТАЕТ (" + String(distance) + " см)");
        return true;
      }
    }
    delay(50);
  }
  
  Serial.println("❌ " + sensorName + " - НЕТ СИГНАЛА (проверьте питание и провода)");
  return false;
}

void checkSensors() {
  Serial.println("--- Проверка датчиков ---");
  
  sensor1OK = checkSensor(trigPin1, echoPin1, "Датчик 1");
  sensor2OK = checkSensor(trigPin2, echoPin2, "Датчик 2");
  
  if (sensor1OK && sensor2OK) {
    Serial.println("✅ ОБА ДАТЧИКА РАБОТАЮТ НОРМАЛЬНО");
  } else {
    Serial.println("⚠️ ВНИМАНИЕ: Проблемы с датчиками!");
  }
  Serial.println("------------------------");
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
  
  if (millis() - lastSensorCheck > SENSOR_CHECK_INTERVAL) {
    checkSensors();
    lastSensorCheck = millis();
  }
  
  if (pumpOn) {
    if (millis() - pumpStartTime >= PUMP_DURATION) {
      digitalWrite(relayPin, HIGH); 
      pumpOn = false;
      Serial.println(">>> НАСОС ВЫКЛ (2 секунды прошло) <<<");
    }
    delay(100);
    return;
  }
  
  long dist1 = readDistance(trigPin1, echoPin1);
  long dist2 = readDistance(trigPin2, echoPin2);
  
  bool hand1 = (dist1 <= maxDistance && dist1 > 0);
  bool hand2 = (dist2 <= maxDistance && dist2 > 0);
  
  if (hand1 && hand2) {
    digitalWrite(relayPin, LOW);
    pumpOn = true;
    pumpStartTime = millis();
    Serial.println(">>> ОБЕ РУКИ - НАСОС ВКЛ НА 2 СЕКУНДЫ <<<");
  }
  
  static long lastDist1 = -1, lastDist2 = -1;
  static bool lastPumpState = false;
  
  if (dist1 != lastDist1 || dist2 != lastDist2 || pumpOn != lastPumpState) {
    Serial.print("Д1:");
    Serial.print(dist1);
    Serial.print("см Д2:");
    Serial.print(dist2);
    Serial.print("см Насос:");
    Serial.println(pumpOn ? "ВКЛ" : "ВЫКЛ");
    
    lastDist1 = dist1;
    lastDist2 = dist2;
    lastPumpState = pumpOn;
  }
  
  delay(200);
}