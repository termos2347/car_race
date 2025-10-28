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
        
        // УПРОЩЕННЫЙ ТЕСТ
        Serial.println("🧪 ТЕСТ СЕРВОПРИВОДА:");
        
        Serial.println("➡️  0°");
        elevatorServo.write(0);
        delay(800);
        
        Serial.println("➡️  90° (нейтраль)");
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
    
    Serial.println("✅ ServoManager готов");
}

void ServoManager::safeServoWrite(int angle) {
    if (!servoAttached) {
        return;
    }
    
    angle = constrain(angle, 0, 180);
    elevatorServo.write(angle);
}

void ServoManager::update(const ControlData& data) {
    // УЛУЧШЕННОЕ ПРЕОБРАЗОВАНИЕ ДЖОЙСТИКА В УГОЛ СЕРВО
    int angle = 90; // Нейтральное положение по умолчанию
    
    // Преобразуем значения джойстика (-512 до 512) в угол (0 до 180)
    // Учитываем, что макс значение джойстика = 180°, мин = 0°
    if (data.yAxis1 != 0) {
        // Нормализуем значение джойстика от -1.0 до 1.0
        float normalized = (float)data.yAxis1 / 512.0f;
        
        // Ограничиваем диапазон от -1.0 до 1.0
        normalized = constrain(normalized, -1.0f, 1.0f);
        
        // Преобразуем в угол: -1.0 → 0°, 0 → 90°, 1.0 → 180°
        angle = (int)(90.0f + (normalized * 90.0f));
        
        // Гарантируем границы
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
    
    // УЛУЧШЕННАЯ ДИАГНОСТИКА (раз в 800 мс)
    static unsigned long lastServoDebug = 0;
    if (millis() - lastServoDebug > 800) {
        // Определяем направление движения
        const char* direction;
        if (angle < 85) direction = "⬇️ ВНИЗ";
        else if (angle > 95) direction = "⬆️ ВВЕРХ";
        else direction = "⏹️ НЕЙТР";
        
        // Статус мотора
        const char* motorStatus = (motorPWM > 0) ? "🚀 ВКЛ" : "⏹️ ВЫКЛ";
        
        Serial.printf("📊 SERVO: %s угол=%-3d° (Y1=%-4d) | %s PWM=%-3d (X1=%-4d)\n", 
                     direction, angle, data.yAxis1, motorStatus, motorPWM, data.xAxis1);
        lastServoDebug = millis();
    }
    
    // ДОПОЛНИТЕЛЬНАЯ ДИАГНОСТИКА ПРИ КРАЙНИХ ПОЛОЖЕНИЯХ
    static int lastAngle = -1;
    if (abs(angle - lastAngle) > 30) { // Только при значительных изменениях
        if (angle <= 5 || angle >= 175) {
            Serial.printf("🎯 КРАЙНЕЕ ПОЛОЖЕНИЕ: %d° (Y1=%d)\n", angle, data.yAxis1);
        }
        lastAngle = angle;
    }
}

void ServoManager::quickTest() {
    Serial.println("🧪 БЫСТРЫЙ ТЕСТ СЕРВОПРИВОДА...");
    
    if (!servoAttached) {
        Serial.println("❌ Сервопривод не подключен");
        return;
    }
    
    // ТОЛЬКО 0, 90, 180
    int testAngles[] = {0, 90, 180, 90};
    const char* positions[] = {"МИН (0°)", "НЕЙТР (90°)", "МАКС (180°)", "ВОЗВРАТ (90°)"};
    
    for (int i = 0; i < 4; i++) {
        Serial.printf("➡️  %s\n", positions[i]);
        safeServoWrite(testAngles[i]);
        delay(700);
    }
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

// Остальные функции без изменений
void ServoManager::quickCalibrate() {
    Serial.println("🎯 Калибровка: серво=90°, мотор=0");
    safeServoWrite(90);
    ledcWrite(MOTOR_CHANNEL, 0);
    delay(1000);
}