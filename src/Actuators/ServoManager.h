#pragma once
#include <ESP32Servo.h>
#include "Core/Types.h"

class ServoManager {
public:
    void begin();
    void update(const ControlData& data);
    void quickCalibrate();
    void emergencyStop();
    void quickTest();
    void testSequence();
    void setServoAngle(int angle);
    void setMotorSpeed(int speed);
    bool isServoAttached();
    void diagnostic();

private:
    Servo elevatorServo;
    bool servoAttached = false;
    
    // Константы
    static const uint8_t ELEVATOR_PIN = HardwareConfig::ELEVATOR_PIN;
    static const uint8_t MOTOR_PIN = HardwareConfig::MOTOR_PIN;
    
    static const int MOTOR_CHANNEL = 0;
    static const int MOTOR_FREQ = 1000;
    static const int MOTOR_RESOLUTION = 8;
    
    static const int SG90_MIN_PULSE = 500;
    static const int SG90_MAX_PULSE = 2400;
    
    static const int JOYSTICK_DEADZONE = 50;
    
    // Приватные методы
    void safeServoWrite(int angle);
};