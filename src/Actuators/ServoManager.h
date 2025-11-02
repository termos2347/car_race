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
    
    // ⚙️ НАСТРОЙКИ СЕРВОПРИВОДА - легко менять здесь!
    // Пины теперь берутся из HardwareConfig в Types.h
    static const int SERVO_MIN_ANGLE = 0;      // Минимальный угол для РАБОТЫ
    static const int SERVO_MAX_ANGLE = 180;    // Максимальный угол для РАБОТЫ
    static const int SERVO_NEUTRAL_ANGLE = 90; // Нейтральное положение
    
    // ⚡ НАСТРОЙКИ ТЕСТИРОВАНИЯ - используем одинаковые значения для всего теста!
    static const int SERVO_TEST_MIN = 0;       // Мин. угол для теста
    static const int SERVO_TEST_MAX = 180;     // Макс. угол для теста
    
    // Настройки PWM для сервопривода
    static const int SERVO_MIN_PULSE = 500;   // Минимальная ширина импульса
    static const int SERVO_MAX_PULSE = 2400;  // Максимальная ширина импульса
};