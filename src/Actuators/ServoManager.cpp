#include "ServoManager.h"
#include <Arduino.h>

void ServoManager::begin() {
    Serial.println("🔧 Инициализация ServoManager...");
    
    if (elevatorServo.attach(ELEVATOR_PIN, SG90_MIN_PULSE, SG90_MAX_PULSE)) {
        Serial.println("✅ Сервопривод SG90 инициализирован");
        elevatorServo.write(90); // Нейтральное положение
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
    // Y1: -512 → 0°, 0 → 90°, 512 → 180°
    
    int angle = map(data.yAxis1, -512, 512, 0, 180);
    angle = constrain(angle, 0, 180);
    
    // НЕМЕДЛЕННО применяем угол
    elevatorServo.write(angle);
    
    // Двигатель от X1
    int motorPWM = 0;
    if (data.xAxis1 > 50) {
        motorPWM = map(data.xAxis1, 50, 512, 80, 255);
    }
    motorPWM = constrain(motorPWM, 0, 255);
    ledcWrite(MOTOR_CHANNEL, motorPWM);
    
    // Диагностика каждого обновления
    Serial.printf("🎯 Y1:%-4d → угол:%-3d° | X1:%-4d → PWM:%-3d\n", 
                 data.yAxis1, angle, data.xAxis1, motorPWM);
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
    
    elevatorServo.write(90);
    Serial.println("✅ Тест соответствия завершен");
}