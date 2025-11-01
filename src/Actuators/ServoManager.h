#pragma once
#include <ESP32Servo.h>
#include "Core/Types.h"

class ServoManager {
public:
    ServoManager();
    void begin();
    void update(const ControlData& data);
    
    // УБИРАЕМ ВСЕ ЛИШНИЕ МЕТОДЫ
    // void quickCalibrate();
    // void emergencyStop();
    // void quickTest();
    // void testSequence();
    // void setServoAngle(int angle);
    // void setMotorSpeed(int speed);
    // bool isServoAttached();
    // void diagnostic();
    // void debugJoystick(const ControlData& data);

private:
    Servo elevatorServo;
    
    static const uint8_t ELEVATOR_PIN = 12;
    static const uint8_t MOTOR_PIN = 13;
};