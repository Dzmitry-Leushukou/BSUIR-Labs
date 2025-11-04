const int relayPin = 2;

void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Выключить реле при старте
  
  Serial.begin(9600);
  Serial.println("=== ТЕСТ РЕЛЕ ===");
  Serial.println("Реле должно щелкать каждые 3 секунды");
  Serial.println("Слушай щелчки!");
}

void loop() {
  // Включаем реле
  digitalWrite(relayPin, HIGH);
  Serial.println("РЕЛЕ: ВКЛ (должен быть щелчок)");
  delay(3000);
  
  // Выключаем реле
  digitalWrite(relayPin, LOW);
  Serial.println("РЕЛЕ: ВЫКЛ (должен быть щелчок)");
  delay(3000);
}