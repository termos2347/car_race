#include "ServoManager.h"
#include <Arduino.h>

ServoManager::ServoManager() {
    // Конструктор - ничего не делаем
}

void ServoManager::begin() {
    Serial.begin(115200);
    Serial.println("🚀 START ServoManager");
    
    // ПРОСТАЯ инициализация сервопривода
    elevatorServo.attach(ELEVATOR_PIN, 500, 2400);
    elevatorServo.write(90);
    delay(1000);
    
    Serial.println("✅ Servo INIT OK");
}

void ServoManager::update(const ControlData& data) {
    // САМАЯ ПРОСТАЯ ЛОГИКА В МИРЕ
    
    // 1. Берем значение джойстика
    int y = data.yAxis1;
    
    // 2. Немедленно преобразуем в угол сервопривода
    int angle = map(y, -512, 512, 0, 180);
    
    // 3. Ограничиваем (на всякий случай)
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    
    // 4. НЕМЕДЛЕННО отправляем на сервопривод
    elevatorServo.write(angle);
    
    // 5. Выводим в Serial для отладки
    Serial.print("Y:");
    Serial.print(y);
    Serial.print(" -> ");
    Serial.print(angle);
    Serial.println("°");
    
    // 6. Мотор пока не трогаем
}