// Пины для датчика
const int trigPin = 3;
const int echoPin = 4;

void setup() {
  // Инициализация датчика
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Инициализация Serial порта
  Serial.begin(9600);
  Serial.println("Проверка датчика HC-SR04");
  Serial.println("=========================");
}

long readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}

void loop() {
  long distance = readDistance();
  
  // Вывод в монитор порта
  Serial.print("Расстояние: ");
  Serial.print(distance);
  Serial.println(" см");
  
  // Проверка работы
  if (distance < 10) {
    Serial.println(">>> РУКА РЯДОМ! <<<");
  } else if (distance > 200) {
    Serial.println("--- НЕТ ОБЪЕКТА ---");
  }
  
  delay(500);
}