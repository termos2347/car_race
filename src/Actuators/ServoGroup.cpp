#include "ServoGroup.h"
#include <Arduino.h>

ServoGroup::ServoGroup(uint8_t pin, int minAngle, int maxAngle, int neutralAngle, const char* name)
    : pin(pin), minAngle(minAngle), maxAngle(maxAngle), neutralAngle(neutralAngle), name(name) {
}

void ServoGroup::begin() {
    Serial.print("🚀 INIT ");
    Serial.print(name);
    Serial.println(" Servo");
    
    servo.attach(pin, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    servo.write(neutralAngle);
    delay(500);
}

void ServoGroup::write(int angle) {
    angle = constrain(angle, minAngle, maxAngle);
    servo.write(angle);
}

void ServoGroup::testToNeutral() {
    servo.write(neutralAngle);
}

void ServoGroup::testToMin() {
    servo.write(minAngle);
}

void ServoGroup::testToMax() {
    servo.write(maxAngle);
}

void ServoGroup::testSequence() {
    isTesting = true;
    
    // Нейтральное положение
    testToNeutral();
    delay(1000);
    
    // Минимальный угол
    testToMin();
    delay(500);
    
    // Максимальный угол
    testToMax();
    delay(500);
    
    // Возврат в нейтральное положение
    testToNeutral();
    delay(500);
    
    isTesting = false;
}