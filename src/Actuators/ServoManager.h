#pragma once
#include <ESP32Servo.h>
#include "../Core/Types.h"

class ServoManager {
public:
    void begin();
    void update(const ControlData& data);
    void quickCalibrate();
    void emergencyStop();
    void testSequence();  
    
private:
    Servo elevatorServo;
    
    static const uint8_t ELEVATOR_PIN = HardwareConfig::ELEVATOR_PIN;
    static const uint8_t MOTOR_PIN = HardwareConfig::MOTOR_PIN;
    
    static const int MOTOR_CHANNEL = 0;
    static const int MOTOR_FREQ = 1000;
    static const int MOTOR_RESOLUTION = 8;
    
    // НАСТРОЙКИ ДЛЯ SG90
    static const int SG90_MIN_PULSE = 500;
    static const int SG90_MAX_PULSE = 2500;
    
    // ПАРАМЕТРЫ ДЛЯ ПЛАВНОГО УПРАВЛЕНИЯ
    static const int JOYSTICK_DEAD_ZONE = 20;  // Мертвая зона джойстика
    static const int MAX_ANGLE_STEP = 5;       // Максимальное изменение угла за шаг
    static const int MOTOR_DEAD_ZONE = 50;     // Мертвая зона двигателя
    static const int MOTOR_MIN_PWM = 80;       // Минимальный PWM для двигателя
    
    // Защита от слишком частых команд
    static const unsigned long MIN_UPDATE_INTERVAL = 30;
    
    unsigned long lastUpdateTime = 0;
    int currentAngle = 90;
};