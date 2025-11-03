const int relayPin = 2;

void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Насос выключен
  
  Serial.begin(9600);
  Serial.println("=== ТЕСТ НАСОСА ===");
  Serial.println("Насос будет включаться на 3 секунды");
}

void loop() {
  // Включаем насос на 3 секунды
  Serial.println(">>> НАСОС ВКЛЮЧЕН <<<");
  digitalWrite(relayPin, HIGH);
  delay(3000);
  
  // Выключаем насос на 7 секунд
  Serial.println(">>> НАСОС ВЫКЛЮЧЕН <<<");
  digitalWrite(relayPin, LOW);
  delay(7000);
}