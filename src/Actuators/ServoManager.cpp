#include "ServoManager.h"
#include <Arduino.h>

ServoManager::ServoManager() {
    // Конструктор - ничего не делаем
}

void ServoManager::begin() {
    Serial.println("🚀 START ServoManager");
    
    // Инициализация сервопривода с настройками из .h
    // Пин теперь берется из HardwareConfig
    elevatorServo.attach(HardwareConfig::ELEVATOR_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
    
    // Запускаем тестовую последовательность
    testSequence();
    
    // Возвращаем в нейтральное положение после теста
    elevatorServo.write(SERVO_NEUTRAL_ANGLE);
    delay(500);
    
    Serial.println("✅ Servo INIT OK on pin " + String(HardwareConfig::ELEVATOR_PIN));
}

void ServoManager::testSequence() {
    Serial.println("🧪 START Servo Test Sequence");
    isTesting = true;
    
    // 1. Нейтральное положение
    Serial.println("➡️ Moving to NEUTRAL (" + String(SERVO_NEUTRAL_ANGLE) + "°)");
    elevatorServo.write(SERVO_NEUTRAL_ANGLE);
    delay(1000);
    
    // 2. Плавное движение к минимальному углу ТЕСТА
    Serial.println("⬇️ Moving to TEST MIN (" + String(SERVO_TEST_MIN) + "°)");
    for (int pos = SERVO_NEUTRAL_ANGLE; pos >= SERVO_TEST_MIN; pos -= 5) {
        elevatorServo.write(pos);
        delay(50);
    }
    delay(500);
    
    // 3. Плавное движение к максимальному углу ТЕСТА
    Serial.println("⬆️ Moving to TEST MAX (" + String(SERVO_TEST_MAX) + "°)");
    for (int pos = SERVO_TEST_MIN; pos <= SERVO_TEST_MAX; pos += 5) {
        elevatorServo.write(pos);
        delay(50);
    }
    delay(500);
    
    // 4. Возврат через нейтральное положение
    Serial.println("↩️ Returning through NEUTRAL");
    for (int pos = SERVO_TEST_MAX; pos >= SERVO_NEUTRAL_ANGLE; pos -= 5) {
        elevatorServo.write(pos);
        delay(50);
    }
    delay(500);
    
    // 5. Быстрая проверка отклика - ИСПОЛЬЗУЕМ ТЕ ЖЕ ЗНАЧЕНИЯ ТЕСТА!
    Serial.println("⚡ Quick response test");
    Serial.println("   Moving to TEST MIN: " + String(SERVO_TEST_MIN) + "°");
    elevatorServo.write(SERVO_TEST_MIN);
    delay(300);
    
    Serial.println("   Moving to TEST MAX: " + String(SERVO_TEST_MAX) + "°");
    elevatorServo.write(SERVO_TEST_MAX);
    delay(300);
    
    Serial.println("   Returning to NEUTRAL: " + String(SERVO_NEUTRAL_ANGLE) + "°");
    elevatorServo.write(SERVO_NEUTRAL_ANGLE);
    delay(300);
    
    Serial.println("✅ Servo Test COMPLETE");
    isTesting = false;
}

void ServoManager::update(const ControlData& data) {
    // Если идет тестирование - игнорируем входящие данные
    if (isTesting) {
        Serial.println("⏸️ Servo update paused during testing");
        return;
    }
    
    // САМАЯ ПРОСТАЯ ЛОГИКА В МИРЕ
    
    // 1. Берем значение джойстика
    int y = data.yAxis1;
    
    // 2. Преобразуем в угол сервопривода с безопасными пределами из констант
    int angle = map(y, -512, 512, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    
    // 3. Ограничиваем используя константы класса
    angle = constrain(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    
    // 4. НЕМЕДЛЕННО отправляем на сервопривод
    elevatorServo.write(angle);
    
    // 5. Выводим в Serial для отладки (редко, чтобы не спамить)
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 500) {
        Serial.print("🎮 Y:");
        Serial.print(y);
        Serial.print(" -> 📐 ");
        Serial.print(angle);
        Serial.print("° [WORKING: ");
        Serial.print(SERVO_MIN_ANGLE);
        Serial.print("-");
        Serial.print(SERVO_MAX_ANGLE);
        Serial.print("] [TEST: ");
        Serial.print(SERVO_TEST_MIN);
        Serial.print("-");
        Serial.print(SERVO_TEST_MAX);
        Serial.print("] ");
        lastPrint = millis();
    }
}