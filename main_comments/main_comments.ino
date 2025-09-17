#include <GyverStepper.h>  // Библиотека для управления шаговым двигателем
#include <Servo.h>         // Библиотека для управления сервоприводами

Servo part0;               // Объект сервопривода 0
Servo part1;               // Объект сервопривода 1
Servo part2;               // Объект сервопривода 2
Servo part3;               // Объект сервопривода 3

Servo servos[] = {part0, part1, part2, part3};              // Массив объектов сервоприводов
int servosCurrentPos[] = {90, 90, 90, 90};                  // Текущие позиции сервоприводов
int servosTargetPos[] = {90, 90, 90, 90};                   // Целевые позиции сервоприводов
uint32_t servosTimer[] = {0, 0, 0, 0};                      // Таймеры для обновления позиций
int sDelay = 30;                                            // Базовая задержка между обновлениями
uint32_t servosDelay[] = {sDelay, sDelay, sDelay, sDelay};  // Задержки для каждого сервопривода

#define NUMB_OF_SERVOS 3                                    // Количество сервоприводов (0-3)
GStepper<STEPPER2WIRE> stepper(800, 3, 4, 2);              // Шаговый двигатель: шагов/оборот, пины STEP, DIR, EN

String inputString = "";                                    // Входящая строка данных
bool stringComplete = false;                                // Флаг завершения приема строки
bool do_not_move = 0;                                       // Флаг режима управления сервоприводами
int number = 0;                                             // Номер сервопривода
int degree = 0;                                             // Целевой угол сервопривода
const float MM_TO_STEPS = 33.5;                             // Коэффициент преобразования мм в шаги

void setup() {
  Serial.begin(115200);                                     // Инициализация Serial порта
  
  stepper.setRunMode(FOLLOW_POS);                           // Режим следования к целевой позиции
  stepper.setMaxSpeed(15000);                               // Максимальная скорость (шагов/сек)
  stepper.autoPower(1);                                     // Автоматическое управление питанием
  stepper.setAcceleration(10000);                           // Ускорение (шагов/сек²)
  
  inputString.reserve(10);                                  // Резервирование памяти для строки
  
  part0.attach(9);                                          // Подключение сервопривода 0 к пину 9
  part1.attach(10);                                         // Подключение сервопривода 1 к пину 10
  part2.attach(11);                                         // Подключение сервопривода 2 к пину 11
  part3.attach(6);                                          // Подключение сервопривода 3 к пину 6
}

void loop() {
  if (stringComplete) {                                     // Если получена полная строка
    processSerialData();                                    // Обработка данных
    inputString = "";                                       // Очистка строки
    stringComplete = false;                                 // Сброс флага
  }
  
  stepper.tick();                                           // Управление шаговым двигателем
  servoPosControl();                                        // Управление сервоприводами
}

void processSerialData() {
  int value = inputString.toInt();                          // Преобразование строки в число
  
  if (do_not_move == 0) {                                   // Режим управления шаговым двигателем
    if (value == 0) {                                       // Команда выключения двигателя
      stepper.autoPower(1);                                 // Отключение питания двигателя
      stepper.setTarget(1, RELATIVE);                       // Минимальное движение для отключения
      Serial.println("Motor power OFF");                    // Сообщение о выключении
      
    } else if (value >= 1 && value <= 250) {                // Команда движения (1-250 мм)
      long targetSteps = value * MM_TO_STEPS;               // Перевод мм в шаги
      stepper.setTarget(targetSteps);                       // Установка целевой позиции
      stepper.autoPower(0);                                 // Включение питания двигателя
      Serial.print("Moving to: ");                          // Вывод информации
      Serial.print(value);
      Serial.print(" mm (");
      Serial.print(targetSteps);
      Serial.println(" steps)");
      
    } else if (value >= 252 && value <= 255) {              // Команда выбора сервопривода (252-255)
      do_not_move = 1;                                      // Переход в режим управления сервоприводом
      Serial.print("do_not_move = 1");                      // Информационное сообщение
      number = value - 252;                                 // Вычисление номера сервопривода
      Serial.print("   selected servo = ");                 // Вывод номера сервопривода
      Serial.println(number);
    }
    
  } else if (do_not_move == 1) {                            // Режим управления сервоприводом
    if(value < 181 && value > -1)                           // Проверка допустимого диапазона (0-180)
      degree = value;                                       // Установка целевого угла
    Serial.print("Servo number ");                          // Вывод информации
    Serial.print(number);
    Serial.print(" turns to ");
    Serial.println(degree);
    do_not_move = 0;                                        // Возврат в режим управления двигателем
    Serial.println("do_not_move = 0");                      // Информационное сообщение
    servosTargetPos[number] = degree;                       // Установка целевой позиции сервопривода
  }
}

void servoPosControl() {
  for (int i = 0; i <= NUMB_OF_SERVOS; i++) {               // Обход всех сервоприводов
    if (millis() - servosTimer[i] > servosDelay[i]) {       // Проверка времени задержки
      // Вычисление направления движения (-1, 0, 1)
      int delta = servosCurrentPos[i] == servosTargetPos[i] ? 0 : 
                 (servosCurrentPos[i] < servosTargetPos[i] ? 1 : -1);
      servosCurrentPos[i] += delta;                         // Обновление текущей позиции
      servosTimer[i] = millis();                            // Сброс таймера
      servos[i].write(servosCurrentPos[i]);                 // Установка позиции сервопривода
    }
  }
}

void serialEvent() {
  while (Serial.available()) {                              // Пока есть данные в порту
    char inChar = (char)Serial.read();                      // Чтение символа
    if (inChar == '\n') {                                   // Если символ новой строки
      stringComplete = true;                                // Установка флага завершения
    } else {
      inputString += inChar;                                // Добавление символа к строке
    }
  }
}