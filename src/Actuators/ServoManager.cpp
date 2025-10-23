#include "ServoManager.h"
#include <Arduino.h>

void ServoManager::begin() {
    Serial.println("🔧 Инициализация ServoManager...");
    
    if (elevatorServo.attach(ELEVATOR_PIN)) {
        Serial.println("✅ Сервопривод инициализирован");
        // Устанавливаем нейтральное положение
        elevatorServo.write(90);
        delay(500);
    } else {
        Serial.println("❌ Ошибка инициализации сервопривода!");
        return;
    }
    
    // Инициализация двигателя
    ledcSetup(MOTOR_CHANNEL, MOTOR_FREQ, MOTOR_RESOLUTION);
    ledcAttachPin(MOTOR_PIN, MOTOR_CHANNEL);
    ledcWrite(MOTOR_CHANNEL, 0);
    
    Serial.println("✅ ServoManager готов");
}

void ServoManager::update(const ControlData& data) {
    // ПРОСТАЯ И НАДЕЖНАЯ ЛОГИКА
    
    // 1. Управление сервоприводом от X1
    int targetAngle = map(data.yAxis1, -512, 512, 0, 180);
    targetAngle = constrain(targetAngle, 0, 180);
    
    // НЕМЕДЛЕННО применяем угол
    elevatorServo.write(targetAngle);
    
    // 2. Управление двигателем от Y1
    int motorPWM = 0;
    if (data.yAxis1 > 50) {
        motorPWM = map(data.xAxis1, 50, 512, 80, 255);
    }
    motorPWM = constrain(motorPWM, 0, 255);
    ledcWrite(MOTOR_CHANNEL, motorPWM);
    
    // Минимальная диагностика (только при значительных изменениях)
    static int lastAngle = -1;
    static int lastPWM = -1;
    
    if (abs(targetAngle - lastAngle) > 5 || motorPWM != lastPWM) {
        Serial.printf("🎯 X1:%-4d → угол:%-3d° | Y1:%-4d → PWM:%-3d\n", 
                     data.xAxis1, targetAngle, data.yAxis1, motorPWM);
        lastAngle = targetAngle;
        lastPWM = motorPWM;
    }
}

void ServoManager::quickCalibrate() {
    elevatorServo.write(90);
    ledcWrite(MOTOR_CHANNEL, 0);
}

void ServoManager::emergencyStop() {
    elevatorServo.write(90);
    ledcWrite(MOTOR_CHANNEL, 0);
}

void ServoManager::testSequence() {
    Serial.println("🎯 ТЕСТ: Проверка сервопривода...");
    
    // Простой тест
    elevatorServo.write(0);
    delay(1000);
    elevatorServo.write(180);
    delay(1000);
    elevatorServo.write(90);
    delay(1000);
    
    Serial.println("✅ Тест завершен");
}