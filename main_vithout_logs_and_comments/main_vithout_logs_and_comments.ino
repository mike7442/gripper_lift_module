#include <GyverStepper.h>
#include <Servo.h>
Servo part0;
Servo part1;
Servo part2;
Servo part3;
Servo servos[] = {part0, part1, part2, part3};
int servosCurrentPos[] = {90, 90, 90, 90};
int servosTargetPos[] = {90, 90, 90, 90};
uint32_t servosTimer[] = {0, 0, 0, 0};
int sDelay = 30;
uint32_t servosDelay[] = {sDelay, sDelay, sDelay, sDelay};
#define NUMB_OF_SERVOS 3
GStepper<STEPPER2WIRE> stepper(800, 3, 4, 2);
String inputString = "";
bool stringComplete = false;
bool do_not_move = 0;
int number = 0;
int degree = 0;
const float MM_TO_STEPS = 33.5;

void setup() {
  Serial.begin(115200);
  stepper.setRunMode(FOLLOW_POS);
  stepper.setMaxSpeed(15000);
  stepper.autoPower(1);
  stepper.setAcceleration(10000);
  inputString.reserve(10);
  part0.attach(9);
  part1.attach(10);
  part2.attach(11);
  part3.attach(6);
}

void loop() {
  if (stringComplete) {
    processSerialData();
    inputString = "";
    stringComplete = false;
  }
  stepper.tick();
  servoPosControl();
}

void processSerialData() {
  int value = inputString.toInt();
  if (do_not_move == 0) {
    if (value == 0) {
      stepper.autoPower(1);
      stepper.setTarget(1, RELATIVE);
    } else if (value >= 1 && value <= 250) {
      long targetSteps = value * MM_TO_STEPS;
      stepper.setTarget(targetSteps);
      stepper.autoPower(0);
    } else if (value >= 252 && value <= 255) {
      do_not_move = 1;
      number = value - 252;
    }
  } else if (do_not_move == 1) {
    if(value < 181 && value > -1)
      degree = value;
    do_not_move = 0;
    servosTargetPos[number] = degree;
  }
}

void servoPosControl() {
  for (int i = 0; i <= NUMB_OF_SERVOS; i++) {
    if (millis() - servosTimer[i] > servosDelay[i]) {
      int delta = servosCurrentPos[i] == servosTargetPos[i] ? 0 : (servosCurrentPos[i] < servosTargetPos[i] ? 1 : -1);
      servosCurrentPos[i] += delta;
      servosTimer[i] = millis();
      servos[i].write(servosCurrentPos[i]);
    }
  }
}

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