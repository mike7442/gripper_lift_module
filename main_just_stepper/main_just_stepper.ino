#include <GyverStepper.h>

GStepper<STEPPER2WIRE> stepper(800, 3, 4, 2);

// Переменные для UART
String inputString = "";
bool stringComplete = false;

// Коэффициент преобразования мм в шаги (настройте под вашу механику)
const float MM_TO_STEPS = 16.0; // 1 мм = 160 шагов (пример)

void setup() {
  Serial.begin(115200);
  
  // Режим следования к целевой позиции
  stepper.setRunMode(FOLLOW_POS);
  
  // Установка макс. скорости в шагах/сек
  stepper.setMaxSpeed(15000);
  
  // Автоматическое управление питанием
  stepper.autoPower(1);
  
  // Установка ускорения в шагах/сек/сек
  stepper.setAcceleration(10000);
  
  // Резервируем память для строки
  inputString.reserve(10);
}

void loop() {
  // Обработка входящих данных
  if (stringComplete) {
    processSerialData();
    inputString = "";
    stringComplete = false;
  }
  
  // Выполняем движение мотора
  stepper.tick();
}

// Функция обработки серийных данных
void processSerialData() {
  // Преобразуем строку в число
  int value = inputString.toInt();
  
  if (value == 0) {
    // Выключаем питание мотора
    stepper.autoPower(1); // Отключаем питание
    stepper.setTarget(1,RELATIVE);
    Serial.println("Motor power OFF");

  } 
  else if (value >= 1 && value <= 255) {
    // Включаем питание и устанавливаем позицию

    // Преобразуем миллиметры в шаги
    long targetSteps = value * MM_TO_STEPS;
    
    // Устанавливаем целевую позицию
    stepper.setTarget(targetSteps);
    stepper.autoPower(0);
    Serial.print("Moving to: ");
    Serial.print(value);
    Serial.print(" mm (");
    Serial.print(targetSteps);
    Serial.println(" steps)");
  }
  else {
    Serial.println("Invalid value! Use 0-255");
  }
}

// Обработчик прерывания серийного порта
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
}
