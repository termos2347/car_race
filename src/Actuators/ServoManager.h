#pragma once
#include <ESP32Servo.h>
#include "../Core/Types.h"  // Включаем наши типы

class ServoManager {
public:
    void begin();
    void update(const ControlData& data);
    void setReverse(uint8_t servoIndex, bool reverse);
    void calibrate();
    
private:
    struct ServoConfig {
        Servo servo;
        uint8_t pin;
        bool reversed;
        uint16_t minPulse;
        uint16_t maxPulse;
    };
    
    ServoConfig servos[8]; // Теперь 8 сервоприводов
    
    void applyMixer(const ControlData& data, int16_t outputs[8]);
    int mapToPulse(int16_t value, bool reverse);
    
    // Инициализация конфигурации сервоприводов
    void initializeServos();
};