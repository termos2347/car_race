#include "ServoManager.h"
#include <Arduino.h>

void ServoManager::initializeServos() {
    // Инициализация сервоприводов для первого джойстика
    servos[0] = {Servo(), 12, false, 1000, 2000};
    servos[1] = {Servo(), 27, false, 1000, 2000};
    servos[2] = {Servo(), 13, false, 1000, 2000};
    servos[3] = {Servo(), 14, false, 1000, 2000};

    // Инициализация сервоприводов для второго джойстика
    servos[4] = {Servo(), 15, false, 1000, 2000};
    servos[5] = {Servo(), 16, false, 1000, 2000};
    servos[6] = {Servo(), 17, false, 1000, 2000};
    servos[7] = {Servo(), 18, false, 1000, 2000};
}

void ServoManager::begin() {
    #if DEBUG_MODE
        Serial.println("🔧 Инициализация сервоприводов...");
    #endif
    
    initializeServos();
    
    // Разрешаем выделение таймеров для ESP32
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    
    for(int i = 0; i < 8; i++) {
        servos[i].servo.setPeriodHertz(50);
        servos[i].servo.attach(servos[i].pin, 
                              servos[i].minPulse, 
                              servos[i].maxPulse);
        servos[i].servo.write(90); // Нейтральное положение
        delay(100);
    }
    
    #if DEBUG_MODE
        Serial.println("✅ Сервоприводы инициализированы (8 шт)");
    #endif
}

void ServoManager::update(const ControlData& data) {
    int16_t outputs[8];
    applyMixer(data, outputs);
    
    for (int i = 0; i < 8; i++) {
        servos[i].servo.writeMicroseconds(mapToPulse(outputs[i], servos[i].reversed));
    }
}

void ServoManager::applyMixer(const ControlData& data, int16_t outputs[8]) {
    int16_t x1 = constrain(data.xAxis1, -512, 512);
    int16_t y1 = constrain(data.yAxis1, -512, 512);
    int16_t x2 = constrain(data.xAxis2, -512, 512);
    int16_t y2 = constrain(data.yAxis2, -512, 512);

    outputs[0] = y1;
    outputs[1] = y1;
    outputs[2] = x1;
    outputs[3] = x1;

    outputs[4] = y2;
    outputs[5] = y2;
    outputs[6] = x2;
    outputs[7] = x2;
}

int ServoManager::mapToPulse(int16_t value, bool reverse) {
    if (reverse) {
        value = -value;
    }
    return map(value, -512, 512, 1000, 2000);
}

void ServoManager::setReverse(uint8_t servoIndex, bool reverse) {
    if (servoIndex < 8) {
        servos[servoIndex].reversed = reverse;
    }
}

void ServoManager::calibrate() {
    #if DEBUG_MODE
        Serial.println("🎯 Калибровка сервоприводов...");
    #endif
    
    for(int i = 0; i < 8; i++) {
        servos[i].servo.writeMicroseconds(1500);
    }
    delay(1000);
    
    #if DEBUG_MODE
        Serial.println("✅ Калибровка завершена (8 сервоприводов)");
    #endif
}

// Новая функция для быстрой калибровки без вывода
void ServoManager::quickCalibrate() {
    for(int i = 0; i < 8; i++) {
        servos[i].servo.writeMicroseconds(1500);
    }
    delay(100);
}