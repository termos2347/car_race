#include "ServoManager.h"
#include <Arduino.h>

void ServoManager::begin() {
    Serial.println("🔧 Инициализация ServoManager...");
    
    // Инициализация таймеров для ESP32Servo
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    
    // Инициализация сервопривода
    elevatorServo.setPeriodHertz(50);
    
    if (elevatorServo.attach(ELEVATOR_PIN, SG90_MIN_PULSE, SG90_MAX_PULSE)) {
        servoAttached = true;
        Serial.println("✅ Сервопривод SG90 инициализирован");
        
        // Тест сервопривода
        Serial.println("🧪 ТЕСТ СЕРВОПРИВОДА:");
        
        Serial.println("➡️  0°");
        elevatorServo.write(0);
        delay(800);
        
        Serial.println("➡️  90°");
        elevatorServo.write(90);
        delay(800);
        
        Serial.println("➡️  180°");
        elevatorServo.write(180);
        delay(800);
        
        // Возврат в нейтральное положение
        elevatorServo.write(90);
        delay(500);
        
        Serial.println("✅ Тест завершен");
    } else {
        Serial.println("❌ ОШИБКА: Сервопривод не инициализирован!");
        servoAttached = false;
        return;
    }
    
    // Инициализация мотора
    ledcSetup(MOTOR_CHANNEL, MOTOR_FREQ, MOTOR_RESOLUTION);
    ledcAttachPin(MOTOR_PIN, MOTOR_CHANNEL);
    ledcWrite(MOTOR_CHANNEL, 0);
    Serial.println("✅ Мотор инициализирован");
    
    Serial.println("🎯 ServoManager готов");
}

void ServoManager::safeServoWrite(int angle) {
    if (!servoAttached) return;
    
    // Гарантируем границы
    angle = constrain(angle, 0, 180);
    
    // УБРАН ДЕБАГ - он мешает быстродействию
    elevatorServo.write(angle);
}

void ServoManager::update(const ControlData& data) {
    static int lastProcessedAngle = 90;
    static unsigned long lastUpdate = 0;
    int targetAngle = 90; // Нейтральное положение по умолчанию

    // Прямое преобразование без инверсии
    if (abs(data.yAxis1) > JOYSTICK_DEADZONE) {
        float ratio = (data.yAxis1 + 512) / 1024.0f;
        targetAngle = ratio * 180;
        targetAngle = constrain(targetAngle, 0, 180);
    }
    
    // Ограничиваем скорость изменения угла
    //int angleDiff = targetAngle - lastProcessedAngle;
    //if (abs(angleDiff) > 10) {
    //    targetAngle = lastProcessedAngle + (angleDiff > 0 ? 10 : -10);
    //}

    safeServoWrite(targetAngle);
    lastProcessedAngle = targetAngle;        
    
    
    // Управление мотором (ось X)
    int motorPWM = 0;
    if (abs(data.xAxis1) > JOYSTICK_DEADZONE) {
        motorPWM = map(abs(data.xAxis1), JOYSTICK_DEADZONE, 512, 0, 255);
    }
    motorPWM = constrain(motorPWM, 0, 255);
    ledcWrite(MOTOR_CHANNEL, motorPWM);
    
    // Диагностика
    if (millis() - lastUpdate > 10) {
        Serial.printf("SERVO: Y1=%-4d -> Angle=%d° | MOTOR: X1=%-4d -> PWM=%d\n", 
                     data.yAxis1, targetAngle, data.xAxis1, motorPWM);
        lastUpdate = millis();
    }
}

void ServoManager::quickCalibrate() {
    Serial.println("🎯 Калибровка: серво=90°, мотор=0");
    safeServoWrite(90);
    ledcWrite(MOTOR_CHANNEL, 0);
    delay(1000);
}

void ServoManager::quickTest() {
    Serial.println("🧪 БЫСТРЫЙ ТЕСТ СЕРВОПРИВОДА...");
    
    if (!servoAttached) {
        Serial.println("❌ Сервопривод не подключен");
        return;
    }
    
    int testAngles[] = {0, 90, 180, 90};
    for (int i = 0; i < 4; i++) {
        safeServoWrite(testAngles[i]);
        Serial.printf("➡️  %d°\n", testAngles[i]);
        delay(600);
    }
    Serial.println("✅ Тест завершен");
}

void ServoManager::emergencyStop() {
    Serial.println("🛑 АВАРИЙНАЯ ОСТАНОВКА!");
    safeServoWrite(90);
    ledcWrite(MOTOR_CHANNEL, 0);
}

void ServoManager::testSequence() {
    Serial.println("🎯 ПОЛНЫЙ ТЕСТ СЕРВОПРИВОДА И МОТОРА...");
    
    if (!servoAttached) {
        Serial.println("❌ Сервопривод не подключен");
        return;
    }
    
    // Тест сервопривода
    int testAngles[] = {0, 45, 90, 135, 180, 90};
    for (int i = 0; i < 6; i++) {
        safeServoWrite(testAngles[i]);
        Serial.printf("📍 Угол: %d°\n", testAngles[i]);
        delay(800);
    }
    
    // Тест мотора
    int pwmValues[] = {0, 100, 180, 255, 0};
    for (int i = 0; i < 5; i++) {
        ledcWrite(MOTOR_CHANNEL, pwmValues[i]);
        Serial.printf("🚀 PWM: %d/255\n", pwmValues[i]);
        delay(1000);
    }
    
    // Возврат в безопасное состояние
    safeServoWrite(90);
    ledcWrite(MOTOR_CHANNEL, 0);
    Serial.println("✅ Полный тест завершен");
}

void ServoManager::setServoAngle(int angle) {
    if (!servoAttached) return;
    angle = constrain(angle, 0, 180);
    safeServoWrite(angle);
    Serial.printf("🎯 Установлен угол: %d°\n", angle);
}

void ServoManager::setMotorSpeed(int speed) {
    speed = constrain(speed, 0, 255);
    ledcWrite(MOTOR_CHANNEL, speed);
    Serial.printf("🚀 Установлена скорость: %d/255\n", speed);
}

bool ServoManager::isServoAttached() {
    return servoAttached;
}

void ServoManager::diagnostic() {
    Serial.println("📊 ДИАГНОСТИКА SERVOMANAGER:");
    Serial.printf("   Сервопривод: %s\n", servoAttached ? "ПОДКЛЮЧЕН" : "ОТКЛЮЧЕН");
    Serial.printf("   Пин сервопривода: %d\n", ELEVATOR_PIN);
    Serial.printf("   Пин мотора: %d\n", MOTOR_PIN);
    Serial.printf("   Канал PWM: %d\n", MOTOR_CHANNEL);
    Serial.printf("   Deadzone: %d\n", JOYSTICK_DEADZONE);
}