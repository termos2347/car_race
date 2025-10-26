#include "ServoManager.h"
#include <Arduino.h>

void ServoManager::begin() {
    Serial.println("🔧 Инициализация ServoManager...");
    
    if (elevatorServo.attach(ELEVATOR_PIN, SG90_MIN_PULSE, SG90_MAX_PULSE)) {
        Serial.println("✅ Сервопривод SG90 инициализирован");
        elevatorServo.write(90);
        delay(500);
    } else {
        Serial.println("❌ Ошибка инициализации сервопривода!");
        return;
    }
    
    ledcSetup(MOTOR_CHANNEL, MOTOR_FREQ, MOTOR_RESOLUTION);
    ledcAttachPin(MOTOR_PIN, MOTOR_CHANNEL);
    ledcWrite(MOTOR_CHANNEL, 0);
    
    Serial.println("✅ ServoManager готов");
}

void ServoManager::update(const ControlData& data) {
    // ПРЯМОЕ ПРЕОБРАЗОВАНИЕ: Джойстик → Сервопривод
    int angle = map(data.yAxis1, -512, 512, 0, 180);
    angle = constrain(angle, 0, 180);
    
    elevatorServo.write(angle);
    
    // Двигатель от X1
    int motorPWM = 0;
    if (data.xAxis1 > 50) {
        motorPWM = map(data.xAxis1, 50, 512, 80, 255);
    }
    motorPWM = constrain(motorPWM, 0, 255);
    ledcWrite(MOTOR_CHANNEL, motorPWM);
}

void ServoManager::quickCalibrate() {
    Serial.println("🎯 Быстрая калибровка: серво=90°, мотор=0");
    elevatorServo.write(90);
    ledcWrite(MOTOR_CHANNEL, 0);
    delay(1000);
}

void ServoManager::emergencyStop() {
    static unsigned long lastEmergencyCall = 0;
    unsigned long currentTime = millis();
    
    // Защита от частых вызовов
    if (currentTime - lastEmergencyCall < 1000) {
        return;
    }
    lastEmergencyCall = currentTime;
    
    Serial.println("🛑 АВАРИЙНАЯ ОСТАНОВКА!");
    elevatorServo.write(90); // Нейтральное положение
    ledcWrite(MOTOR_CHANNEL, 0); // Выключить мотор
    
    // Мигание для индикации аварийного режима
    for (int i = 0; i < 3; i++) {
        digitalWrite(2, HIGH);
        delay(100);
        digitalWrite(2, LOW);
        delay(100);
    }
}

void ServoManager::testSequence() {
    Serial.println("🎯 ТЕСТ: Проверка соответствия джойстик→серво...");
    
    // Тест точного соответствия
    int testValues[] = {-512, -256, 0, 256, 512};
    const char* positions[] = {"MIN", "25%", "CENTER", "75%", "MAX"};
    
    for (int i = 0; i < 5; i++) {
        int angle = map(testValues[i], -512, 512, 0, 180);
        elevatorServo.write(angle);
        Serial.printf("📍 %s: Y1=%-4d → угол=%-3d°\n", positions[i], testValues[i], angle);
        delay(1500);
    }
    
    // Тест мотора
    Serial.println("🔧 Тест мотора...");
    for (int pwm = 0; pwm <= 255; pwm += 50) {
        ledcWrite(MOTOR_CHANNEL, pwm);
        Serial.printf("🚀 PWM: %d\n", pwm);
        delay(1000);
    }
    ledcWrite(MOTOR_CHANNEL, 0);
    
    elevatorServo.write(90);
    Serial.println("✅ Тест завершен");
}