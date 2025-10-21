#include "ServoManager.h"
#include <Arduino.h>

void ServoManager::initializeServos() {
    // Инициализация сервоприводов для первого джойстика
    servos[0] = {Servo(), 12, false, 1000, 2000}; // Servo 0 - Y ось (джойстик 1)
    servos[1] = {Servo(), 27, false, 1000, 2000}; // Servo 1 - Y ось (джойстик 1)
    servos[2] = {Servo(), 13, false, 1000, 2000}; // Servo 2 - X ось (джойстик 1)
    servos[3] = {Servo(), 14, false, 1000, 2000}; // Servo 3 - X ось (джойстик 1)

    // Инициализация сервоприводов для второго джойстика
    servos[4] = {Servo(), 15, false, 1000, 2000}; // Servo 4 - Y ось (джойстик 2)
    servos[5] = {Servo(), 16, false, 1000, 2000}; // Servo 5 - Y ось (джойстик 2)
    servos[6] = {Servo(), 17, false, 1000, 2000}; // Servo 6 - X ось (джойстик 2)
    servos[7] = {Servo(), 18, false, 1000, 2000}; // Servo 7 - X ось (джойстик 2)
}

void ServoManager::begin() {
    Serial.println("🔧 Инициализация сервоприводов...");
    
    initializeServos();
    
    // Разрешаем выделение таймеров для ESP32
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    
    for(int i = 0; i < 8; i++) {
        servos[i].servo.setPeriodHertz(50); // Стандартная частота серв
        servos[i].servo.attach(servos[i].pin, 
                              servos[i].minPulse, 
                              servos[i].maxPulse);
        servos[i].servo.write(90); // Нейтральное положение
        delay(100);
    }
    
    Serial.println("✅ Сервоприводы инициализированы (8 шт)");
}

void ServoManager::update(const ControlData& data) {
    int16_t outputs[8];
    applyMixer(data, outputs);
    
    for (int i = 0; i < 8; i++) {
        servos[i].servo.writeMicroseconds(mapToPulse(outputs[i], servos[i].reversed));
    }
}

void ServoManager::applyMixer(const ControlData& data, int16_t outputs[8]) {
    // Первый джойстик управляет сервоприводами 0-3
    // Второй джойстик управляет сервоприводами 4-7

    // Простое преобразование без микширования
    // Для первого джойстика:
    // X ось: вперед-назад -> сервоприводы 2 и 3
    // Y ось: влево-вправо -> сервоприводы 0 и 1

    // Для второго джойстика:
    // X ось: вперед-назад -> сервоприводы 6 и 7
    // Y ось: влево-вправо -> сервоприводы 4 и 5

    // Ограничение значений
    int16_t x1 = constrain(data.xAxis1, -512, 512);
    int16_t y1 = constrain(data.yAxis1, -512, 512);
    int16_t x2 = constrain(data.xAxis2, -512, 512);
    int16_t y2 = constrain(data.yAxis2, -512, 512);

    // Первый джойстик
    outputs[0] = y1; // Сервопривод 0 - Y ось
    outputs[1] = y1; // Сервопривод 1 - Y ось
    outputs[2] = x1; // Сервопривод 2 - X ось
    outputs[3] = x1; // Сервопривод 3 - X ось

    // Второй джойстик
    outputs[4] = y2; // Сервопривод 4 - Y ось
    outputs[5] = y2; // Сервопривод 5 - Y ось
    outputs[6] = x2; // Сервопривод 6 - X ось
    outputs[7] = x2; // Сервопривод 7 - X ось
}

int ServoManager::mapToPulse(int16_t value, bool reverse) {
    if (reverse) {
        value = -value;
    }
    
    // Преобразование -512..512 в 1000..2000 мкс
    return map(value, -512, 512, 1000, 2000);
}

void ServoManager::setReverse(uint8_t servoIndex, bool reverse) {
    if (servoIndex < 8) {
        servos[servoIndex].reversed = reverse;
    }
}

void ServoManager::calibrate() {
    Serial.println("🎯 Калибровка сервоприводов...");
    
    // Установка нейтрального положения
    for(int i = 0; i < 8; i++) {
        servos[i].servo.writeMicroseconds(1500);
    }
    delay(1000);
    
    Serial.println("✅ Калибровка завершена (8 сервоприводов)");
}