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
    static const int SERVO_MIN_ANGLE = 0;      // Минимальный угол для РАБОТЫ
    static const int SERVO_MAX_ANGLE = 180;    // Максимальный угол для РАБОТЫ
    static const int SERVO_NEUTRAL_ANGLE = 90; // Нейтральное положение
    static const int SERVO_TEST_MIN = 0;       // Мин. угол для теста (должно совпадать с SERVO_MIN_ANGLE если нужны одинаковые)
    static const int SERVO_TEST_MAX = 180;     // Макс. угол для теста (должно совпадать с SERVO_MAX_ANGLE если нужны одинаковые)
    
    // Настройки PWM для сервопривода
    static const int SERVO_MIN_PULSE = 500;   // Минимальная ширина импульса
    static const int SERVO_MAX_PULSE = 2400;  // Максимальная ширина импульса
};