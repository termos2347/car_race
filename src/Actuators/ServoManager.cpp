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
        
        // УПРОЩЕННЫЙ ТЕСТ - ТОЛЬКО 0, 90, 180
        Serial.println("🧪 ТЕСТ СЕРВОПРИВОДА:");
        
        Serial.println("➡️  0°");
        elevatorServo.write(0);
        delay(1000);
        
        Serial.println("➡️  90° (нейтраль)");
        elevatorServo.write(90);
        delay(1000);
        
        Serial.println("➡️  180°");
        elevatorServo.write(180);
        delay(1000);
        
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
    
    Serial.println("✅ ServoManager готов");
}

void ServoManager::safeServoWrite(int angle) {
    if (!servoAttached) {
        return;
    }
    
    angle = constrain(angle, 0, 180);
    elevatorServo.write(angle);
    
    // УБРАН ВЫВОД ЛОГОВ - только в основном update
}

void ServoManager::update(const ControlData& data) {
    // ПРЕОБРАЗОВАНИЕ ДАННЫХ ДЖОЙСТИКА В УГОЛ СЕРВОПРИВОДА
    int angle = 90; // Нейтральное положение по умолчанию
    
    // Применяем deadzone для устранения дрожания
    if (data.yAxis1 < -JOYSTICK_DEADZONE || data.yAxis1 > JOYSTICK_DEADZONE) {
        angle = map(data.yAxis1, -512, 512, 0, 180);
        angle = constrain(angle, 0, 180);
    }
    
    // Управление сервоприводом
    safeServoWrite(angle);
    
    // Управление мотором с deadzone
    int motorPWM = 0;
    if (data.xAxis1 > JOYSTICK_DEADZONE) {
        motorPWM = map(data.xAxis1, JOYSTICK_DEADZONE, 512, 80, 255);
        motorPWM = constrain(motorPWM, 0, 255);
    }
    ledcWrite(MOTOR_CHANNEL, motorPWM);
    
    // УПРОЩЕННЫЙ ВЫВОД ДИАГНОСТИКИ (раз в 1000 мс)
    static unsigned long lastServoDebug = 0;
    if (millis() - lastServoDebug > 1000) {
        // Определяем статус джойстиков
        const char* yStatus = (abs(data.yAxis1) <= JOYSTICK_DEADZONE) ? "⏹️" : "🎯";
        const char* xStatus = (data.xAxis1 <= JOYSTICK_DEADZONE) ? "⏹️" : "🚀";
        
        Serial.printf("📊 SERVO: %s Y1=%-4d → %3d° | %s X1=%-4d → PWM=%-3d\n", 
                     yStatus, data.yAxis1, angle, xStatus, data.xAxis1, motorPWM);
        lastServoDebug = millis();
    }
}

void ServoManager::quickTest() {
    Serial.println("🧪 БЫСТРЫЙ ТЕСТ СЕРВОПРИВОДА...");
    
    if (!servoAttached) {
        Serial.println("❌ Сервопривод не подключен");
        return;
    }
    
    // ТОЛЬКО 0, 90, 180
    Serial.println("➡️  0°");
    safeServoWrite(0);
    delay(800);
    
    Serial.println("➡️  90°");
    safeServoWrite(90);
    delay(800);
    
    Serial.println("➡️  180°");
    safeServoWrite(180);
    delay(800);
    
    // Возврат в нейтральное положение
    safeServoWrite(90);
    Serial.println("✅ Тест завершен");
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
    safeServoWrite(90); // Нейтральное положение
    ledcWrite(MOTOR_CHANNEL, 0); // Выключить мотор
}

void ServoManager::testSequence() {
    Serial.println("🎯 ПОЛНЫЙ ТЕСТ СЕРВОПРИВОДА И МОТОРА...");
    
    if (!servoAttached) {
        Serial.println("❌ Сервопривод не подключен - тест невозможен");
        return;
    }
    
    // Подробный тест сервопривода
    int testAngles[] = {0, 45, 90, 135, 180, 90};
    const char* angleNames[] = {"MIN (0°)", "45°", "NEUTRAL (90°)", "135°", "MAX (180°)", "NEUTRAL"};
    
    for (int i = 0; i < 6; i++) {
        safeServoWrite(testAngles[i]);
        Serial.printf("📍 %s: угол=%d°\n", angleNames[i], testAngles[i]);
        delay(2000);
    }
    
    // Тест мотора
    Serial.println("🔧 Тест мотора...");
    int pwmValues[] = {0, 100, 180, 255, 0};
    for (int i = 0; i < 5; i++) {
        ledcWrite(MOTOR_CHANNEL, pwmValues[i]);
        Serial.printf("🚀 PWM: %d/255\n", pwmValues[i]);
        delay(1500);
    }
    
    // Возврат в безопасное состояние
    safeServoWrite(90);
    ledcWrite(MOTOR_CHANNEL, 0);
    Serial.println("✅ Полный тест завершен");
}