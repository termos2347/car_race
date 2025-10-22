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
    
    // Используем пины из центральной конфигурации
    static const uint8_t ELEVATOR_PIN = HardwareConfig::ELEVATOR_PIN;
    static const uint8_t MOTOR_PIN = HardwareConfig::MOTOR_PIN;
    
    static const int MOTOR_CHANNEL = 0;
    static const int MOTOR_FREQ = 1000;
    static const int MOTOR_RESOLUTION = 8;
    
    // Параметры для плавного управления
    static const int MOTOR_RAMP_UP_SPEED = 2;
    static const int MOTOR_RAMP_DOWN_SPEED = 4;
    static const int MOTOR_DEAD_ZONE = 50;
    static const int MOTOR_MIN_PWM = 80;
    
    // Границы сервопривода (микросекунды)
    static const int SERVO_MIN_PULSE = 1000;
    static const int SERVO_MAX_PULSE = 2000;
    static const int SERVO_NEUTRAL_PULSE = 1500;
    
    // Границы входных значений
    static const int INPUT_MIN = -512;
    static const int INPUT_MAX = 512;
    
    void applyMixer(const ControlData& data, int16_t& servoOutput, int16_t& motorOutput);
    int mapToPulse(int16_t value);
    int mapToMotorPWM(int16_t value);
    bool validatePins();
    void testDirectPWM();  // Добавляем объявление приватного метода
    
    // Для плавного изменения мощности двигателя
    int currentMotorPWM = 0;
};