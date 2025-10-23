#include "ServoManager.h"
#include <Arduino.h>

void ServoManager::begin() {
    Serial.println("🔧 Инициализация ServoManager...");
    
    if (elevatorServo.attach(ELEVATOR_PIN, SG90_MIN_PULSE, SG90_MAX_PULSE)) {
        Serial.println("✅ Сервопривод SG90 инициализирован");
        elevatorServo.write(90);
        currentAngle = 90;
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
    // УПРАВЛЕНИЕ СЕРВОПРИВОДОМ ОТ Y1
    
    // 1. Применяем мертвую зону для центрального положения
    int processedY1 = data.yAxis1;
    if (abs(processedY1) < JOYSTICK_DEAD_ZONE) {
        processedY1 = 0;
    }
    
    // 2. Преобразуем значение джойстика в угол
    int targetAngle = map(processedY1, -512, 512, 0, 180);
    targetAngle = constrain(targetAngle, 0, 180);
    
    // 3. Плавное изменение угла (ограничение скорости)
    int angleDiff = targetAngle - currentAngle;
    if (abs(angleDiff) > MAX_ANGLE_STEP) {
        currentAngle += (angleDiff > 0) ? MAX_ANGLE_STEP : -MAX_ANGLE_STEP;
    } else {
        currentAngle = targetAngle;
    }
    
    // 4. Применяем угол к сервоприводу
    elevatorServo.write(currentAngle);
    
    // 5. Управление двигателем от X1
    int motorPWM = 0;
    if (data.xAxis1 > MOTOR_DEAD_ZONE) {
        motorPWM = map(data.xAxis1, MOTOR_DEAD_ZONE, 512, MOTOR_MIN_PWM, 255);
    }
    motorPWM = constrain(motorPWM, 0, 255);
    ledcWrite(MOTOR_CHANNEL, motorPWM);
    
    // 6. Диагностика только при значительных изменениях
    static int lastPrintedAngle = -1;
    static int lastPrintedPWM = -1;
    
    if (abs(currentAngle - lastPrintedAngle) > 2 || abs(motorPWM - lastPrintedPWM) > 10) {
        Serial.printf("🎯 Y1:%-4d → обраб:%-4d → угол:%-3d° | X1:%-4d → PWM:%-3d\n", 
                     data.yAxis1, processedY1, currentAngle, data.xAxis1, motorPWM);
        lastPrintedAngle = currentAngle;
        lastPrintedPWM = motorPWM;
    }
}

void ServoManager::quickCalibrate() {
    elevatorServo.write(90);
    currentAngle = 90;
    ledcWrite(MOTOR_CHANNEL, 0);
    delay(100);
}

void ServoManager::emergencyStop() {
    elevatorServo.write(90);
    currentAngle = 90;
    ledcWrite(MOTOR_CHANNEL, 0);
}

void ServoManager::testSequence() {
    Serial.println("🎯 ТЕСТ: Проверка промежуточных положений...");
    
    // Тест всех промежуточных положений
    int testAngles[] = {0, 30, 60, 90, 120, 150, 180, 150, 120, 90, 60, 30, 0};
    
    for (int i = 0; i < 13; i++) {
        elevatorServo.write(testAngles[i]);
        currentAngle = testAngles[i];
        Serial.printf("📍 Угол: %d°\n", testAngles[i]);
        delay(1000);
    }
    
    // Возврат в нейтраль
    elevatorServo.write(90);
    currentAngle = 90;
    
    Serial.println("✅ Тест промежуточных положений завершен");
}