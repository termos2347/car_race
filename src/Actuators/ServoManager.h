#pragma once
#include <ESP32Servo.h>
#include "Core/Types.h"

class ServoManager {
public:
    ServoManager();
    void begin();
    void update(const ControlData& data);
    void testSequence();
    
private:
    Servo elevatorServo;
    bool isTesting = false;
    
    // Конфигурационные константы (легко настраивать!)
    static const uint8_t ELEVATOR_PIN = 12;
    static const uint8_t MOTOR_PIN = 13;
    
    // ⚙️ НАСТРОЙКИ СЕРВОПРИВОДА - легко менять здесь!
    static const int SERVO_MIN_ANGLE = 60;    // Минимальный угол (безопасность)
    static const int SERVO_MAX_ANGLE = 120;   // Максимальный угол (безопасность) 
    static const int SERVO_NEUTRAL_ANGLE = 90; // Нейтральное положение
    static const int SERVO_TEST_MIN = 60;     // Мин. угол для теста
    static const int SERVO_TEST_MAX = 120;    // Макс. угол для теста
    
    // Настройки PWM для сервопривода (если нужно настроить)
    static const int SERVO_MIN_PULSE = 500;   // Минимальная ширина импульса
    static const int SERVO_MAX_PULSE = 2400;  // Максимальная ширина импульса
};