#include "ServoManager.h"
#include <Arduino.h>

void ServoManager::begin() {
    Serial.println("🔧 Детальная инициализация ServoManager...");
    Serial.print("📍 Сервопривод на пине: "); Serial.println(ELEVATOR_PIN);
    Serial.print("📍 Мотор на пине: "); Serial.println(MOTOR_PIN);
    
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
        
        // ПРОСТОЙ И НАДЕЖНЫЙ ТЕСТ
        Serial.println("🧪 ТЕСТ СЕРВОПРИВОДА:");
        Serial.println("➡️  Угол 0°");
        elevatorServo.write(0);
        delay(1500);
        
        Serial.println("➡️  Угол 45°");
        elevatorServo.write(45);
        delay(1500);
        
        Serial.println("➡️  Угол 90° (нейтраль)");
        elevatorServo.write(90);
        delay(1500);
        
        Serial.println("➡️  Угол 135°");
        elevatorServo.write(135);
        delay(1500);
        
        Serial.println("➡️  Угол 180°");
        elevatorServo.write(180);
        delay(1500);
        
        // Возврат в нейтральное положение
        Serial.println("➡️  Возврат в нейтраль 90°");
        elevatorServo.write(90);
        delay(500);
        
        Serial.println("🎯 Тест завершен - сервопривод готов");
    } else {
        Serial.println("❌ ОШИБКА: Сервопривод не инициализирован!");
        servoAttached = false;
        return;
    }
    
    // Инициализация мотора
    ledcSetup(MOTOR_CHANNEL, MOTOR_FREQ, MOTOR_RESOLUTION);
    ledcAttachPin(MOTOR_PIN, MOTOR_CHANNEL);
    ledcWrite(MOTOR_CHANNEL, 0);
    Serial.println("✅ Мотор инициализирован (PWM канал 0)");
    
    Serial.println("✅ ServoManager готов");
}

void ServoManager::safeServoWrite(int angle) {
    if (!servoAttached) {
        return;
    }
    
    angle = constrain(angle, 0, 180);
    
    // ДИАГНОСТИКА ТОЛЬКО ПРИ ЗНАЧИТЕЛЬНОМ ИЗМЕНЕНИИ УГЛА (>5°)
    static int lastAngle = -1;
    if (abs(angle - lastAngle) > 5) {
        Serial.printf("🛠️  Установка серво: %d° (предыдущий %d°)\n", angle, lastAngle);
        lastAngle = angle;
    }
    
    elevatorServo.write(angle);
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
    
    // ОГРАНИЧЕННЫЙ ВЫВОД ДИАГНОСТИКИ (раз в 1000 мс)
    static unsigned long lastServoDebug = 0;
    if (millis() - lastServoDebug > 1000) {
        Serial.printf("🔧 SERVO: Y1=%-4d → угол=%-3d° | МОТОР: X1=%-4d → PWM=%-3d\n", 
                     data.yAxis1, angle, data.xAxis1, motorPWM);
        lastServoDebug = millis();
    }
}

void ServoManager::quickCalibrate() {
    Serial.println("🎯 Быстрая калибровка: серво=90°, мотор=0");
    safeServoWrite(90);
    ledcWrite(MOTOR_CHANNEL, 0);
    delay(1000);
}

void ServoManager::quickTest() {
    Serial.println("🧪 БЫСТРЫЙ ТЕСТ СЕРВОПРИВОДА...");
    
    if (!servoAttached) {
        Serial.println("❌ Сервопривод не подключен - пропуск теста");
        return;
    }
    
    // Тест крайних положений
    Serial.println("➡️  Тест: 0° (мин)");
    safeServoWrite(0);
    delay(1000);
    
    Serial.println("➡️  Тест: 90° (нейтраль)");
    safeServoWrite(90);
    delay(1000);
    
    Serial.println("➡️  Тест: 180° (макс)");
    safeServoWrite(180);
    delay(1000);
    
    // Возврат в нейтральное положение
    safeServoWrite(90);
    Serial.println("✅ Быстрый тест завершен");
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