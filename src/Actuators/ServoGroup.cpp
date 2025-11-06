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
    currentAngle = neutralAngle;
    delay(500);
}

void ServoGroup::write(int angle) {
    angle = constrain(angle, minAngle, maxAngle);
    servo.write(angle);
    currentAngle = angle;
}

void ServoGroup::writeSmooth(int targetAngle, int movementTime) {
    targetAngle = constrain(targetAngle, minAngle, maxAngle);
    
    if (targetAngle == currentAngle) {
        return;
    }
    
    int angleDifference = abs(targetAngle - currentAngle);
    int steps = angleDifference;
    int stepDelay = movementTime / steps;
    
    stepDelay = constrain(stepDelay, 1, 100);
    
    int step = (targetAngle > currentAngle) ? 1 : -1;
    
    for (int i = 0; i < steps; i++) {
        currentAngle += step;
        servo.write(currentAngle);
        delay(stepDelay);
    }
}

void ServoGroup::testToNeutral() {
    writeSmooth(neutralAngle, 1000);
}

void ServoGroup::testToMin() {
    writeSmooth(minAngle, 1000);
}

void ServoGroup::testToMax() {
    writeSmooth(maxAngle, 1000);
}

void ServoGroup::testSequence() {
    isTesting = true;
    
    testToNeutral();
    delay(1000);
    
    testToMin();
    delay(500);
    
    testToMax();
    delay(500);
    
    testToNeutral();
    delay(500);
    
    isTesting = false;
}