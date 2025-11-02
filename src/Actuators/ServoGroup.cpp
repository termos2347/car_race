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

void ServoGroup::testSequence() {
    isTesting = true;
    Serial.print("🧪 TEST ");
    Serial.println(name);
    
    // Нейтральное положение
    Serial.print("➡️ ");
    Serial.print(name);
    Serial.print(" NEUTRAL (");
    Serial.print(neutralAngle);
    Serial.println("°)");
    servo.write(neutralAngle);
    delay(1000);
    
    // Минимальный угол
    Serial.print("⬇️ ");
    Serial.print(name);
    Serial.print(" MIN (");
    Serial.print(minAngle);
    Serial.println("°)");
    servo.write(minAngle);
    delay(500);
    
    // Максимальный угол
    Serial.print("⬆️ ");
    Serial.print(name);
    Serial.print(" MAX (");
    Serial.print(maxAngle);
    Serial.println("°)");
    servo.write(maxAngle);
    delay(500);
    
    // Возврат в нейтральное положение
    servo.write(neutralAngle);
    delay(500);
    
    isTesting = false;
}